#pragma once
#include <Async/TcpListener.hpp>

#include "Router.hpp"
namespace wf {
struct Context {
  async::Reactor& reactor;
  async::MultiThreadExecutor& executor;
  async::TcpStream& stream;
  std::unique_ptr<ParamsType> params;
};
class Server {
public:
  Server(Routers&& router) : mRouters(std::move(router)) {};

  auto run() -> StdResult<void>
  {
    using async::SocketAddr;
    using async::SocketAddrV4;
    auto listener = async::TcpListener::Bind(mReactor, SocketAddr(SocketAddrV4 {{0, 0, 0, 0}, 8080}));
    if (!listener) {
      return make_unexpected(listener.error());
    } else {
      auto loop = [this](async::TcpListener l) -> Task<> {
        while (true) {
          auto stream = co_await l.accept(nullptr);
          if (!stream) {
            co_return;
          } else {
            auto spawn = [this](async::TcpStream s) -> Task<> {
              auto request = co_await wf::RecvHttpRequest(s);
              if (!request) {
                co_return;
              }
              auto result = mRouters.handle(**request);
              if (!result) {
                co_return;
              } else {
                auto ctx = Context {mReactor, mExecutor, s, std::move(result.value().params)};
                auto task = (result.value().handle)(ctx);
                auto response = co_await std::move(task);
                if (!response) {
                  co_return;
                }
                auto result = co_await wf::SendHttpResponse(s, *response);
                if (!result) {
                  co_return;
                }
              }
              co_return;
            };
            mExecutor.spawnDetach(spawn(std::move(*stream)), mReactor);
          }
        }
        co_return;
      };
      mExecutor.block(loop(std::move(listener).value()), mReactor);
    }
    return {};
  }

private:
  Routers mRouters;
  async::MultiThreadExecutor mExecutor {4};
  async::Reactor mReactor {};
};
} // namespace wf