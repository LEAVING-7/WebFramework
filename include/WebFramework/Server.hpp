#pragma once
#include <Async/TcpListener.hpp>

#include "Context.hpp"
#include "Router.hpp"
namespace wf {

class Server {
public:
  Server(Routers&& router) : mRouters(std::move(router)) {};

  auto run() -> StdResult<void>
  {
    using async::SocketAddr;
    using async::SocketAddrV4;
    auto listener = async::TcpListener::Bind(mReactor, SocketAddr(SocketAddrV4 {{0, 0, 0, 0}, 80}));
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
              auto params = ParamsType {};
              auto result = mRouters.handle(**request, params); // get handler from request
              if (!result) {
                co_return;
              } else {
                auto queries = QueriesType {};
                if (!ParseQueries(request->get()->path, queries)) {
                  co_return;
                };
                auto ctx = Context {
                    mReactor, mExecutor, s, params, queries,
                };
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
  async::MultiThreadExecutor mExecutor {4};
  async::Reactor mReactor {};
};
} // namespace wf
