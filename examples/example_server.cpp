#include <WebFramework/Server.hpp>
int main()
{
  auto routers = wf::Routers {};
  auto root = routers.rootGroup()->GET("/index", [](wf::Context& ctx) -> Task<std::unique_ptr<wf::HttpResponse>> {
    auto response = std::make_unique<wf::HttpResponse>();
    response->status = wf::HttpStatus::Ok;
    response->version = wf::HttpVersion::Http11;
    response->reason = "OK";
    response->body = std::format("Hello");
    co_return response;
  });
  auto v1 = root->newGroup("/v1")
                ->GET("/",
                      [](wf::Context& ctx) -> Task<std::unique_ptr<wf::HttpResponse>> {
                        auto response = std::make_unique<wf::HttpResponse>();
                        response->status = wf::HttpStatus::Ok;
                        response->version = wf::HttpVersion::Http11;
                        response->reason = "OK";
                        response->body = std::format("/v1/");
                        co_return response;
                      })
                ->GET("/hello", [](wf::Context& ctx) -> Task<std::unique_ptr<wf::HttpResponse>> {
                  auto response = std::make_unique<wf::HttpResponse>();
                  response->status = wf::HttpStatus::Ok;
                  response->version = wf::HttpVersion::Http11;
                  response->reason = "OK";
                  response->body = std::format("/v1/hello");
                  co_return response;
                });
  auto v2 = root->newGroup("/v2")
                ->GET("/hello/:name",
                      [](wf::Context& ctx) -> Task<std::unique_ptr<wf::HttpResponse>> {
                        auto response = std::make_unique<wf::HttpResponse>();
                        response->status = wf::HttpStatus::Ok;
                        response->version = wf::HttpVersion::Http11;
                        response->reason = "OK";
                        response->body = std::format("/v2/hello/{}", ctx.params->at("name"));
                        co_return response;
                      })
                ->POST("/login", [](wf::Context& ctx) -> Task<std::unique_ptr<wf::HttpResponse>> {
                  auto response = std::make_unique<wf::HttpResponse>();
                  response->status = wf::HttpStatus::Ok;
                  response->version = wf::HttpVersion::Http11;
                  response->reason = "OK";
                  response->body = std::format("login");
                  co_return response;
                });
  auto server = wf::Server {std::move(routers)};
  server.run();
}