#include <WebFramework/Server.hpp>

// acme http01 challenge
// usage: ./example_acme_http01 <content>
int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cout << "usage: ./example_acme_http01 <content>" << std::endl;
    return 0;
  }
  auto content = std::string_view(argv[1]);
  auto routers = wf::Routers {};
  routers.rootGroup()->get("/.well-known/acme-challenge/:key", [&content](wf::Context& ctx) -> async::Task<bool> {
    ctx.text(content);
    co_return true;
  });
  auto server = wf::Server {std::move(routers), {async::SocketAddrV4 {async::Ipv4Addr::Any, 80}}};
  server.run();
}
