#pragma once
#include <Async/TcpListener.hpp>

#include "Context.hpp"
#include "Router.hpp"
namespace wf {

class Server {
public:
  Server(Routers&& router, async::SocketAddr addr) : mRouters(std::move(router)), mAddr(addr) {};

  auto run() -> StdResult<void>
  {
    auto listener = async::TcpListener::Bind(mReactor, mAddr);
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
              auto ctx = Context {
                  mReactor,
                  mExecutor,
                  s,
                  **request,
              };
              if (!ParseQueries(request->get()->path, ctx.mQueries)) {
                co_return;
              };
              auto result = mRouters.handle(**request, ctx.mParams); // get handler from request
              if (!result) {
                co_return;
              } else {
                ctx.initGroups(this->mRouters, request->get()->path);
                auto task = (result.value())(ctx);
                try {
                  auto response = co_await std::move(task);
                  if (!response) {
                    co_return;
                  }
                  auto result = co_await wf::SendHttpResponse(s, ctx.mResponse);
                  if (!result) {
                    co_return;
                  }
                } catch (std::exception& e) {
                  utils::println("exception: {}", e.what());
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
  async::SocketAddr mAddr;
  async::MultiThreadExecutor mExecutor {4};
  async::Reactor mReactor {};
};
} // namespace wf
