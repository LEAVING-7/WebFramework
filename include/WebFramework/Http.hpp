#pragma once
#include <Async/TcpStream.hpp>
#include <string>
#include <unordered_map>

#include "picohttpparser.h"

namespace wf {
enum class HttpMethod {
  Get,
  Post,
  Put,
  Delete,
  Head,
  Connect,
  Options,
  Trace,
  Patch,
  Unknown,
};
auto ToString(HttpMethod method) -> std::string_view;

enum class HttpVersion {
  Http10,
  Http11,
  Http2,
  Unknown,
};
auto ToString(HttpVersion version) -> std::string_view;

enum class HttpStatus {
  Continue = 100,
  SwitchingProtocols = 101,
  Processing = 102,
  EarlyHints = 103,
  Ok = 200,
  Created = 201,
  Accepted = 202,
  NonAuthoritativeInformation = 203,
  NoContent = 204,
  ResetContent = 205,
  PartialContent = 206,
  MultiStatus = 207,
  AlreadyReported = 208,
  IMUsed = 226,
  MultipleChoices = 300,
  MovedPermanently = 301,
  Found = 302,
  SeeOther = 303,
  NotModified = 304,
  UseProxy = 305,
  SwitchProxy = 306,
  TemporaryRedirect = 307,
  PermanentRedirect = 308,
  BadRequest = 400,
  Unauthorized = 401,
  PaymentRequired = 402,
  Forbidden = 403,
  NotFound = 404,
  MethodNotAllowed = 405,
  NotAcceptable = 406,
  ProxyAuthenticationRequired = 407,
  RequestTimeout = 408,
  Conflict = 409,
  Gone = 410,
  LengthRequired = 411,
  PreconditionFailed = 412,
  PayloadTooLarge = 413,
  URITooLong = 414,
  UnsupportedMediaType = 415,
  RangeNotSatisfiable = 416,
  ExpectationFailed = 417,
  ImATeapot = 418,
  MisdirectedRequest = 421,
  UnprocessableEntity = 422,
  Locked = 423,
  FailedDependency = 424,
  TooEarly = 425,
  UpgradeRequired = 426,
  PreconditionRequired = 428,
  TooManyRequests = 429,
  RequestHeaderFieldsTooLarge = 431,
  UnavailableForLegalReasons = 451,
  InternalServerError = 500,
  NotImplemented = 501,
  BadGateway = 502,
  ServiceUnavailable = 503,
  GatewayTimeout = 504,
  HTTPVersionNotSupported = 505,
  VariantAlsoNegotiates = 506,
  InsufficientStorage = 507,
  LoopDetected = 508,
  NotExtended = 510,
  NetworkAuthenticationRequired = 511,
};
auto ToString(HttpStatus status) -> std::string_view;
// HTTP Request
struct HttpRequest {
  HttpMethod method;
  HttpVersion version;
  std::string path;
  std::unordered_map<std::string, std::string> headers;
  std::string body;
};
auto ToString(HttpRequest const& req) -> std::string;

// HTTP Response
struct HttpResponse {
  HttpVersion version;
  HttpStatus status;
  std::string reason;
  std::unordered_map<std::string, std::string> headers;
  std::string body;
};
auto ToString(HttpResponse const& res) -> std::string;

auto RecvHttpRequest(async::TcpStream& stream) -> Task<StdResult<std::unique_ptr<HttpRequest>>>;
auto RecvHttpResponse(async::TcpStream& stream) -> Task<StdResult<std::unique_ptr<HttpResponse>>>;

auto RecvHttpResponse(async::TcpStream& stream, std::span<char> buffer)
    -> Task<StdResult<std::unique_ptr<HttpResponse>>>;
auto RecvHttpRequest(async::TcpStream& stream, std::span<char> buffer) -> Task<StdResult<std::unique_ptr<HttpRequest>>>;

auto SendHttpRequest(async::TcpStream& stream, HttpRequest const& req) -> Task<StdResult<void>>;
auto SendHttpResponse(async::TcpStream& stream, HttpResponse const& res) -> Task<StdResult<void>>;

} // namespace wf
