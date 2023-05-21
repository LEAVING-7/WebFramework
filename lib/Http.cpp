#include "WebFramework/Http.hpp"
#include <cassert>
#include <format>
namespace wf {
auto HttpResponse::set(HttpStatus status) -> void
{
  this->version = HttpVersion::Http11;
  this->status = status;
  this->reason = ToString(this->status);
}
auto HttpResponse::set(std::underlying_type_t<HttpStatus> status) -> void
{
  return set(static_cast<HttpStatus>(status));
}
auto static ParseHttpMethod(std::string_view method) -> HttpMethod
{
  if (method == "GET") {
    return HttpMethod::Get;
  } else if (method == "POST") {
    return HttpMethod::Post;
  } else if (method == "PUT") {
    return HttpMethod::Put;
  } else if (method == "DELETE") {
    return HttpMethod::Delete;
  } else if (method == "HEAD") {
    return HttpMethod::Head;
  } else if (method == "OPTIONS") {
    return HttpMethod::Options;
  } else if (method == "CONNECT") {
    return HttpMethod::Connect;
  } else if (method == "TRACE") {
    return HttpMethod::Trace;
  } else if (method == "PATCH") {
    return HttpMethod::Patch;
  } else {
    assert(0);
  }
}

auto ToString(HttpMethod method) -> std::string_view
{
  switch (method) {
  case HttpMethod::Get:
    return "GET";
  case HttpMethod::Post:
    return "POST";
  case HttpMethod::Put:
    return "PUT";
  case HttpMethod::Delete:
    return "DELETE";
  case HttpMethod::Head:
    return "HEAD";
  case HttpMethod::Options:
    return "OPTIONS";
  case HttpMethod::Connect:
    return "CONNECT";
  case HttpMethod::Trace:
    return "TRACE";
  case HttpMethod::Patch:
    return "PATCH";
  default:
    assert(0);
  }
}

auto ToString(HttpVersion version) -> std::string_view
{
  switch (version) {
  case HttpVersion::Http10:
    return "HTTP/1.0";
  case HttpVersion::Http11:
    return "HTTP/1.1";
  case HttpVersion::Http2:
    return "HTTP/2";
  default:
    assert(0);
  }
}

auto ToString(HttpStatus status) -> std::string_view
{
  // clang-format off
  switch (status) {
  case HttpStatus::Continue: return "Continue";
  case HttpStatus::SwitchingProtocols: return "Switching Protocols";
  case HttpStatus::Processing: return "Processing";
  case HttpStatus::EarlyHints: return "Early Hints";
  case HttpStatus::Ok: return "OK";
  case HttpStatus::Created: return "Created";
  case HttpStatus::Accepted: return "Accepted";
  case HttpStatus::NonAuthoritativeInformation: return "Non-Authoritative Information";
  case HttpStatus::NoContent: return "No Content";
  case HttpStatus::ResetContent: return "Reset Content"; 
  case HttpStatus::PartialContent: return "Partial Content"; 
  case HttpStatus::MultiStatus: return "Multi-Status"; 
  case HttpStatus::AlreadyReported: return "Already Reported"; 
  case HttpStatus::IMUsed: return "IM Used"; 
  case HttpStatus::MultipleChoices: return "Multiple Choices"; 
  case HttpStatus::MovedPermanently: return "Moved Permanently"; 
  case HttpStatus::Found: return "Found"; 
  case HttpStatus::SeeOther: return "See Other"; 
  case HttpStatus::NotModified: return "Not Modified"; 
  case HttpStatus::UseProxy: return "Use Proxy"; 
  case HttpStatus::SwitchProxy: return "Switch Proxy"; 
  case HttpStatus::TemporaryRedirect: return "Temporary Redirect"; 
  case HttpStatus::PermanentRedirect: return "Permanent Redirect"; 
  case HttpStatus::BadRequest: return "Bad Request"; 
  case HttpStatus::Unauthorized:  return "Unauthorized";
  case HttpStatus::PaymentRequired: return "Payment Required";
  case HttpStatus::Forbidden: return "Forbidden";
  case HttpStatus::NotFound: return "Not Found"; 
  case HttpStatus::MethodNotAllowed: return "Method Not Allowed"; 
  case HttpStatus::NotAcceptable: return "Not Acceptable"; 
  case HttpStatus::ProxyAuthenticationRequired: return "Proxy Authentication Required"; 
  case HttpStatus::RequestTimeout: return "Request Timeout"; 
  case HttpStatus::Conflict: return "Conflict"; 
  case HttpStatus::Gone: return "Gone"; 
  case HttpStatus::LengthRequired: return "Length Required"; 
  case HttpStatus::PreconditionFailed: return "Precondition Failed"; 
  case HttpStatus::PayloadTooLarge: return "Payload Too Large"; 
  case HttpStatus::URITooLong: return "URI Too Long"; 
  case HttpStatus::UnsupportedMediaType: return "Unsupported Media Type"; 
  case HttpStatus::RangeNotSatisfiable: return "Range Not Satisfiable"; 
  case HttpStatus::ExpectationFailed: return "Expectation Failed"; 
  case HttpStatus::ImATeapot: return "I'm a teapot"; 
  case HttpStatus::MisdirectedRequest: return "Misdirected Request"; 
  case HttpStatus::UnprocessableEntity: return "Unprocessable Entity";
  case HttpStatus::Locked: return "Locked";
  case HttpStatus::FailedDependency: return "Failed Dependency";
  case HttpStatus::TooEarly: return "Too Early";
  case HttpStatus::UpgradeRequired: return "Upgrade Required";
  case HttpStatus::PreconditionRequired: return "Precondition Required"; 
  case HttpStatus::TooManyRequests: return "Too Many Requests"; 
  case HttpStatus::RequestHeaderFieldsTooLarge: return "Request Header Fields Too Large"; 
  case HttpStatus::UnavailableForLegalReasons: return "Unavailable For Legal Reasons"; 
  case HttpStatus::InternalServerError: return "Internal Server Error"; 
  case HttpStatus::NotImplemented: return "Not Implemented"; 
  case HttpStatus::BadGateway: return "Bad Gateway"; 
  case HttpStatus::ServiceUnavailable: return "Service Unavailable"; 
  case HttpStatus::GatewayTimeout: return "Gateway Timeout"; 
  case HttpStatus::HTTPVersionNotSupported: return "HTTP Version Not Supported"; 
  case HttpStatus::VariantAlsoNegotiates: return "Variant Also Negotiates"; 
  case HttpStatus::InsufficientStorage: return "Insufficient Storage";
  case HttpStatus::LoopDetected: return "Loop Detected"; 
  case HttpStatus::NotExtended: return "Not Extended";  
  case HttpStatus::NetworkAuthenticationRequired: return "Network Authentication Required";
  default: assert(0);
  }
  // clang-format on
}

auto RecvHttpRequest(async::TcpStream& stream, std::span<char> buffer) -> Task<StdResult<std::unique_ptr<HttpRequest>>>
{
  char const *method, *path;
  int pret, minorVersion;
  struct phr_header headers[100];
  size_t bufLen = 0, prevBufLen = 0, methodLen, pathLen, headerNums = std::size(headers);
  while (true) {
    auto recvNum =
        co_await stream.recv(std::as_writable_bytes(std::span<char>(buffer.data() + bufLen, buffer.size() - bufLen)));
    if (!recvNum) {
      co_return make_unexpected(recvNum.error());
    }
    if (recvNum.value() == 0) {
      co_return make_unexpected(std::make_error_code(std::errc::io_error));
    }

    auto const len = recvNum.value();
    prevBufLen = bufLen;
    bufLen += len;
    pret = phr_parse_request(buffer.data(), bufLen, &method, &methodLen, &path, &pathLen, &minorVersion, headers,
                             &headerNums, prevBufLen);
    if (pret > 0) {
      break;
    } else if (pret == -1) {
      co_return make_unexpected(std::make_error_code(std::errc::invalid_argument));
    }
    assert(pret == -2);
    if (bufLen == buffer.size()) {
      co_return make_unexpected(std::make_error_code(std::errc::no_buffer_space));
    }
  }
  auto req = std::make_unique<HttpRequest>();
  req->method = ParseHttpMethod(std::string_view(method, methodLen));
  req->path = std::string_view(path, pathLen);
  req->version = minorVersion == 0 ? HttpVersion::Http10 : HttpVersion::Http11;
  for (size_t i = 0; i < headerNums; ++i) {
    req->headers.emplace(std::string_view(headers[i].name, headers[i].name_len),
                         std::string_view(headers[i].value, headers[i].value_len));
  }
  req->body = std::string(buffer.data() + pret, bufLen - pret);
  co_return req;
}
auto RecvHttpResponse(async::TcpStream& stream, std::span<char> buf) -> Task<StdResult<std::unique_ptr<HttpResponse>>>
{
  int pret, minorVersion, status;
  struct phr_header headers[100];
  size_t bufLen = 0, prevBufLen = 0, headerNums = std::size(headers);
  char const* msgs;
  size_t msgLen;

  while (true) {
    auto recvNum =
        co_await stream.recv(std::as_writable_bytes(std::span<char>(buf.data() + bufLen, buf.size() - bufLen)));
    if (!recvNum) {
      co_return make_unexpected(recvNum.error());
    }
    auto const len = recvNum.value();
    prevBufLen = bufLen;
    bufLen += len;
    pret = phr_parse_response(buf.data(), bufLen, &minorVersion, &status, &msgs, &msgLen, headers, &headerNums,
                              prevBufLen);
    if (pret > 0) {
      break;
    } else if (pret == -1) {
      co_return make_unexpected(std::make_error_code(std::errc::invalid_argument));
    }
    assert(pret == -2);
    if (bufLen == buf.size()) {
      co_return make_unexpected(std::make_error_code(std::errc::message_size));
    }
  }
  auto res = std::make_unique<HttpResponse>();
  res->version = minorVersion == 0 ? HttpVersion::Http10 : HttpVersion::Http11;
  res->status = static_cast<HttpStatus>(status);
  res->reason = std::string_view(msgs, msgLen);
  for (size_t i = 0; i < headerNums; ++i) {
    res->headers.emplace(std::string_view(headers[i].name, headers[i].name_len),
                         std::string_view(headers[i].value, headers[i].value_len));
  }
  res->body = std::string(buf.data() + pret, bufLen - pret);
  co_return res;
}

auto RecvHttpRequest(async::TcpStream& stream) -> Task<StdResult<std::unique_ptr<HttpRequest>>>
{
  auto buf = std::array<char, 4096>();
  co_return co_await RecvHttpRequest(stream, buf);
}
auto RecvHttpResponse(async::TcpStream& stream) -> Task<StdResult<std::unique_ptr<HttpResponse>>>
{
  auto buf = std::array<char, 4096>();
  co_return co_await RecvHttpResponse(stream, buf);
}

auto SendHttpRequest(async::TcpStream& stream, HttpRequest const& req) -> Task<StdResult<void>>
{
  auto const buf = ToString(req);
  auto sent = size_t(0);
  while (sent < buf.size()) {
    auto const sendNum = co_await stream.send(std::as_bytes(std::span(buf.data() + sent, buf.size() - sent)));
    if (!sendNum) {
      co_return make_unexpected(sendNum.error());
    }
    sent += sendNum.value();
  }
  co_return {};
}
auto SendHttpResponse(async::TcpStream& stream, HttpResponse const& res) -> Task<StdResult<void>>
{
  auto const buf = ToString(res);
  auto sent = size_t(0);
  while (sent < buf.size()) {
    auto const sendNum = co_await stream.send(std::as_bytes(std::span(buf.data() + sent, buf.size() - sent)));
    if (!sendNum) {
      co_return make_unexpected(sendNum.error());
    }
    sent += sendNum.value();
  }
  co_return {};
}
auto ToString(HttpRequest const& req) -> std::string
{
  auto ret = std::string();

  ret += std::format("{} {} {}\r\n", ToString(req.method), req.path, ToString(req.version));
  for (auto const& [name, value] : req.headers) {
    ret += std::format("{}: {}\r\n", name, value);
  }
  ret += "\r\n";
  ret += req.body;
  return ret;
}
auto ToString(HttpResponse const& res) -> std::string
{
  using StatusCodeUnderlyingType = std::underlying_type_t<HttpStatus>;
  auto ret = std::string();
  ret +=
      std::format("{} {} {}\r\n", ToString(res.version), static_cast<StatusCodeUnderlyingType>(res.status), res.reason);
  for (auto const& [name, value] : res.headers) {
    ret += std::format("{}: {}\r\n", name, value);
  }
  ret += "\r\n";
  ret += res.body;
  return ret;
}
} // namespace wf