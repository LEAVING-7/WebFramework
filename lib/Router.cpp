#include "WebFramework/Router.hpp"

namespace wf {
auto RouterGroup::newGroup(std::string_view prefix) -> RouterGroup*
{
  auto g = std::make_unique<RouterGroup>(this->mPrefix + std::string(prefix), this->mRoot, this);
  return mRoot->addGroup(std::move(g));
}

auto RouterGroup::addRoute(HttpMethod method, std::string_view comp, HandlerType&& handle) -> RouterGroup*
{
  auto pattern = mPrefix + std::string(comp);
  mRoot->getRouter().addRoute(method, pattern, std::move(handle));
  return this;
}
auto Routers::addGroup(std::unique_ptr<RouterGroup> group) -> RouterGroup*
{
  this->mGroups.emplace_back(std::move(group));
  return this->mGroups.back().get();
}
auto ParseQueries(std::string_view str, QueriesType& queries) -> bool
{
  if (str.empty()) {
    return true;
  }
  auto pos = str.find('?');
  if (pos == std::string_view::npos) {
    return true;
  }
  str = str.substr(pos + 1);
  auto parts = utils::StringSplit(str, '&');
  for (auto& part : parts) {
    auto kv = utils::StringSplit(part, '=');
    if (kv.size() == 2) {
      queries[kv[0]] = kv[1];
    }
  }
  return true;
}
} // namespace wf