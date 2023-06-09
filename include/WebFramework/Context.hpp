#pragma once

#include "Router.hpp"
#include "json11.hpp"
namespace wf {
class Context {
public:
  friend class Server;
  Context(async::Reactor& r, async::MultiThreadExecutor& e, async::TcpStream& s, HttpRequest& request);
  auto json(json11::Json const& json) -> bool;
  auto html(std::string_view html) -> bool;
  auto text(std::string_view text) -> bool;

  auto reactor() -> async::Reactor& { return mReactor; }
  auto executor() -> async::MultiThreadExecutor& { return mExecutor; }
  auto stream() -> async::TcpStream& { return mStream; }
  auto getParam(std::string const& key) -> std::optional<std::string_view>
  {
    auto it = mParams.find(key);
    if (it == mParams.end()) {
      return std::nullopt;
    }
    return it->second;
  }
  auto getQuery(std::string const& key) -> std::optional<std::string_view>
  {
    auto it = mQueries.find(key);
    if (it == mQueries.end()) {
      return std::nullopt;
    }
    return it->second;
  }
  auto response() -> HttpResponse& { return mResponse; }
  auto isAborted() -> bool { return mIsAborted; }

  auto runMiddleware() -> async::Task<bool>;
  auto runAllMiddleware() -> async::Task<bool>;

  HttpRequest& mRequest;
  QueriesType mQueries;
  ParamsType mParams;
private:
  async::Reactor& mReactor;
  async::MultiThreadExecutor& mExecutor;
  async::TcpStream& mStream;
  HttpResponse mResponse;
  std::vector<RouterGroup*> mGroups;
  uint32_t mGroupIndex;
  bool mIsAborted;
  auto initGroups(Routers& routers, std::string_view path) -> void;
};
} // namespace wf