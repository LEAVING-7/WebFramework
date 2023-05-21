#include <WebFramework/Server.hpp>
int main()
{
  auto routers = wf::Routers {};
  auto root = routers.rootGroup()
                  ->use([](wf::Context& ctx) -> Task<bool> {
                    printf("new connection at fd: %d\n", ctx.stream.getSocket().raw());
                    co_return true;
                  })
                  ->GET("/index", [](wf::Context& ctx) -> Task<bool> {
                    wf::utils::println("before run middleware, ctx address: {}, groupIndex: {}, group size: {}",
                                       (void*)&ctx, ctx.groupIndex, ctx.groups.size());
                    co_await ctx.runAllMiddleware();
                    ctx.response.status = wf::HttpStatus::Ok;
                    ctx.response.version = wf::HttpVersion::Http11;
                    ctx.response.reason = "OK";
                    ctx.response.body = std::format("Hello");
                    co_return true;
                  });
  auto v1 = root->newGroup("/v1")
                ->use([](wf::Context& ctx) -> Task<bool> {
                  wf::utils::println("v1 middleware");
                  co_return true;
                })
                ->GET("/",
                      [](wf::Context& ctx) -> Task<bool> {
                        co_await ctx.runAllMiddleware();
                        ctx.response.status = wf::HttpStatus::Ok;
                        ctx.response.version = wf::HttpVersion::Http11;
                        ctx.response.reason = "OK";
                        ctx.response.body = std::format("/v1/");
                        co_return true;
                      })
                ->GET("/hello", [](wf::Context& ctx) -> Task<bool> {
                  co_await ctx.runAllMiddleware();
                  ctx.response.status = wf::HttpStatus::Ok;
                  ctx.response.version = wf::HttpVersion::Http11;
                  ctx.response.reason = "OK";
                  ctx.response.body = std::format("/v1/hello");
                  throw std::runtime_error("test");
                  co_return true;
                });
  auto v2 = root->newGroup("/v2")
                ->use([](wf::Context& ctx) -> Task<bool> {
                  wf::utils::println("v2 middleware");
                  co_return true;
                })
                ->GET("/hello/:name",
                      [](wf::Context& ctx) -> Task<bool> {
                        co_await ctx.runAllMiddleware();
                        ctx.response.status = wf::HttpStatus::Ok;
                        ctx.response.version = wf::HttpVersion::Http11;
                        ctx.response.reason = "OK";
                        ctx.response.body = std::format("/v2/hello/{}", ctx.params->at("name"));
                        co_return true;
                      })
                ->POST("/login/*username", [](wf::Context& ctx) -> Task<bool> {
                  co_await ctx.runAllMiddleware();
                  ctx.response.status = wf::HttpStatus::Ok;
                  ctx.response.version = wf::HttpVersion::Http11;
                  ctx.response.reason = "OK";
                  ctx.response.body = std::format("login");
                  co_return true;
                });
  auto server = wf::Server {std::move(routers)};
  server.run();
}