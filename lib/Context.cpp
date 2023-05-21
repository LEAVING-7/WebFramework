#include "WebFramework/Context.hpp"

namespace wf {
auto Context::json(json11::Json const& json) -> bool
{
  auto res = json.dump();
  mResponse.set(HttpStatus::Ok);
  mResponse.headers["Content-Type"] = "application/json";
  mResponse.headers["Content-Length"] = std::to_string(res.size());
  mResponse.body = std::move(res);
  return true;
}

auto Context::html(std::string_view html) -> bool
{
  mResponse.set(HttpStatus::Ok);
  mResponse.headers["Content-Type"] = "text/html";
  mResponse.headers["Content-Length"] = std::to_string(html.size());
  mResponse.body = std::string(html);
  return true;
}
auto Context::text(std::string_view text) -> bool
{
  mResponse.set(HttpStatus::Ok);
  mResponse.headers["Content-Type"] = "text/plain";
  mResponse.headers["Content-Length"] = std::to_string(text.size());
  mResponse.body = std::string(text);
  return true;
}

auto Context::runMiddleware() -> Task<bool>
{
  if (mGroupIndex < mGroups.size()) {
    auto group = mGroups[mGroupIndex];
    mGroupIndex++;
    auto handles = group->getMiddleware();
    for (auto&& handle : handles) {
      auto res = co_await handle(*this);
      if (!res) {
        co_return false;
      }
    }
    co_return true;
  }
  co_return true;
}
auto Context::runAllMiddleware() -> Task<bool>
{
  while (mGroupIndex < mGroups.size()) {
    auto res = co_await runMiddleware();
    if (!res) {
      co_return false;
    }
  }
  co_return true;
}
auto Context::initGroups(Routers& routers, std::string_view path) -> void
{
  auto& rtGroups = routers.getGroups();
  mGroups = std::vector<RouterGroup*> {};
  for (auto&& group : rtGroups) {
    auto prefix = group->getPrefix();
    if (path.starts_with(prefix)) {
      mGroups.push_back(group.get());
    }
  }
  std::sort(mGroups.begin(), mGroups.end(),
            [](auto&& a, auto&& b) { return a->getPrefix().size() < b->getPrefix().size(); });
  assert(mGroups.front()->getPrefix() == "");
}
Context::Context(async::Reactor& r, async::MultiThreadExecutor& e, async::TcpStream& s, HttpRequest& request)
    : mRequest(request), mQueries(), mParams(), mReactor(r), mExecutor(e), mStream(s), mResponse(), mGroups(),
      mGroupIndex(0), mIsAborted(false)
{
}
} // namespace wf