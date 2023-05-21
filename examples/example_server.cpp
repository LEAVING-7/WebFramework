#include <WebFramework/Server.hpp>
int main()
{
  auto routers = wf::Routers {};
  auto root = routers.rootGroup()
                  ->use([](wf::Context& ctx) -> Task<bool> {
                    printf("new connection at fd: %d\n", ctx.stream().getSocket().raw());
                    co_return true;
                  })
                  ->get("/index", [](wf::Context& ctx) -> Task<bool> {
                    co_await ctx.runAllMiddleware();
                    ctx.html("<h1>Hello</h1>");
                    co_return true;
                  });
  auto v1 = root->newGroup("/v1")
                ->use([](wf::Context& ctx) -> Task<bool> {
                  wf::utils::println("v1 middleware");
                  co_return true;
                })
                ->get("/",
                      [](wf::Context& ctx) -> Task<bool> {
                        co_await ctx.runAllMiddleware();
                        ctx.html("<h1>Hello world</h1>");
                        co_return true;
                      })
                ->get("/hello", [](wf::Context& ctx) -> Task<bool> {
                  co_await ctx.runAllMiddleware();
                  wf::utils::println("Queries size:{}", ctx.mQueries.size());
                  auto name = ctx.getQuery("name");
                  ctx.text(std::format("Hello {}", name.value()));
                  co_return true;
                });
  auto v2 = root->newGroup("/v2")
                ->use([](wf::Context& ctx) -> Task<bool> {
                  wf::utils::println("v2 middleware");
                  co_return true;
                })
                ->get("/hello/:name",
                      [](wf::Context& ctx) -> Task<bool> {
                        co_await ctx.runAllMiddleware();
                        ctx.text(std::format("Hello {}", ctx.getParam("name").value()));
                        co_return true;
                      })
                ->post("/login/*username", [](wf::Context& ctx) -> Task<bool> {
                  co_await ctx.runAllMiddleware();
                  ctx.text(std::format("Hello {}", ctx.getParam("username").value()));
                  co_return true;
                });
  auto server = wf::Server {std::move(routers)};
  server.run();
}