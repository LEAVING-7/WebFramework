#pragma once

#include "Http.hpp"
#include "utils.hpp"
namespace wf {
struct Context;
class RouterGroup;
class Routers;

using ParamsType = std::map<std::string, std::string>;
using ResponseType = Task<bool>;
using HandlerFnType = ResponseType(Context&);
using HandlerType = std::function<HandlerFnType>;

struct Context {
  friend class Server;
  async::Reactor& reactor;
  async::MultiThreadExecutor& executor;
  async::TcpStream& stream;
  std::unique_ptr<ParamsType> params;
  HttpResponse response;
  // middleware
  std::vector<RouterGroup*> groups;
  uint32_t groupIndex;
  bool isAborted;


  auto runMiddleware() -> Task<bool>;
  auto runAllMiddleware() -> Task<bool>;

private:
  auto initGroups(Routers& routers, std::string_view path) -> void;
};
class Router {
  struct Node {
    std::string pattern; // e.g. /p/:name
    std::string part;    // e.g. :name
    std::vector<Node> children;
    bool isWild;

    auto matchChild(std::string_view part) -> Node*
    {
      for (auto& child : children) {
        if (child.part == part || child.isWild) {
          return &child;
        }
      }
      return nullptr;
    }
    auto matchChildren(std::string_view part) -> std::vector<Node*>
    {
      std::vector<Node*> nodes;
      for (auto& child : children) {
        if (child.part == part || child.isWild) {
          nodes.push_back(&child);
        }
      }
      return nodes;
    }
    auto insert(std::string_view pattern, std::span<std::string> parts, int32_t height) -> void
    {
      if (parts.size() == height) {
        this->pattern = pattern;
        return;
      }
      auto& part = parts[height];
      auto child = matchChild(part);
      if (child == nullptr) {
        auto isWild = part[0] == ':' || part[0] == '*';
        auto node = Node {{}, part, {}, isWild};
        children.push_back(node);
      }
      children.back().insert(pattern, parts, height + 1);
    }
    auto search(std::span<std::string> parts, int32_t height) -> Node*
    {
      if (parts.size() == size_t(height) || part.starts_with('*')) {
        if (pattern.empty()) {
          return nullptr;
        }
        return this;
      }
      auto& part = parts[height];
      auto children = matchChildren(part);
      for (auto child : children) {
        auto result = child->search(parts, height + 1);
        if (result != nullptr) {
          return result;
        }
      }
      return nullptr;
    }
  };

public:
  Router() = default;

  struct Handler {
    std::reference_wrapper<HandlerType> handle;
    std::unique_ptr<ParamsType> params {nullptr};
  };
  auto handle(HttpRequest const& request) -> std::optional<Handler> { return getHandler(request.method, request.path); }
  auto getHandler(HttpMethod method, std::string_view path) -> std::optional<Handler>
  {
    auto [node, params] = getRoute(method, path);
    if (node == nullptr) {
      return std::nullopt;
    }
    auto key = std::string(ToString(method)) + "-" + node->pattern;
    auto it = mHandles.find(key);
    if (it != mHandles.end()) {
      return {Handler {it->second, std::move(params)}};
    }
    return std::nullopt;
  }
  auto GET(std::string_view patten, HandlerType&& handle) -> void
  {
    addRoute(HttpMethod::Get, patten, std::move(handle));
  }
  auto POST(std::string_view patten, HandlerType&& handle) -> void
  {
    addRoute(HttpMethod::Post, patten, std::move(handle));
  }

  auto addRoute(HttpMethod method, std::string_view pattern, HandlerType&& handle) -> void
  {
    auto parts = ParsePattern(pattern);
    auto key = std::string(ToString(method)) + "-" + std::string(pattern);
    if (!mRoots.contains(method)) {
      mRoots[method] = Node {{}, {}, {}, false};
    }
    mRoots[method].insert(pattern, parts, 0);
    mHandles[key] = std::move(handle);
  }
  auto getRoute(HttpMethod method, std::string_view path)
      -> std::pair<Node*, std::unique_ptr<std::map<std::string, std::string>>>
  {
    if (!mRoots.contains(method)) {
      return {nullptr, nullptr};
    }
    auto searchParts = ParsePattern(path);
    auto params = std::make_unique<std::map<std::string, std::string>>();
    auto n = mRoots[method].search(searchParts, 0);
    if (n != nullptr) {
      auto parts = ParsePattern(n->pattern);
      for (size_t i = 0; i < parts.size(); i++) {
        auto part = parts[i];
        if (part[0] == ':') {
          (*params)[part.substr(1)] = searchParts[i];
        }
        if (part[0] == '*' && part.size() > 1) {
          (*params)[part.substr(1)] = utils::StringJoin(std::span(searchParts).subspan(i), "/");
          break;
        }
      }
      return {n, std::move(params)};
    }
    return {nullptr, nullptr};
  }

private:
  static auto ParsePattern(std::string_view pattern) -> std::vector<std::string>
  {
    auto split = utils::StringSplit(pattern, '/');
    auto parts = std::vector<std::string>();
    for (auto&& item : split) {
      if (!item.empty()) {
        parts.push_back(item);
        if (item[0] == '*') {
          break;
        }
      }
    }
    return parts;
  }

private:
  std::map<HttpMethod, Node> mRoots;
  std::map<std::string, HandlerType> mHandles;
};

class RouterGroup {
public:
  RouterGroup() : RouterGroup("", nullptr, nullptr) {};
  RouterGroup(std::string_view prefix, Routers* root, RouterGroup* parent)
      : mPrefix(prefix), mMiddlewareHandles(), mRoot(root), mParent(parent)
  {
  }
  ~RouterGroup() = default;

  auto newGroup(std::string_view prefix) -> RouterGroup*;
  auto getPrefix() const -> std::string_view { return mPrefix; }
  auto use(HandlerType&& handle) -> RouterGroup*
  {
    mMiddlewareHandles.push_back(std::move(handle));
    return this;
  }
  auto getMiddleware() const -> std::span<HandlerType const> { return mMiddlewareHandles; }
  auto getParent() const -> RouterGroup* { return mParent; }
  auto addRoute(HttpMethod method, std::string_view comp, HandlerType&& handle) -> RouterGroup*;
  auto GET(std::string_view comp, HandlerType&& handle) -> RouterGroup*
  {
    return addRoute(HttpMethod::Get, comp, std::move(handle));
  }
  auto POST(std::string_view comp, HandlerType&& handle) -> RouterGroup*
  {
    return addRoute(HttpMethod::Get, comp, std::move(handle));
  }

private:
  std::string mPrefix;
  std::vector<HandlerType> mMiddlewareHandles;
  Routers* mRoot;
  RouterGroup* mParent;
};

class Routers {
public:
  Routers() = default;
  Routers(Routers&&) = default;
  ~Routers() = default;
  auto addGroup(std::unique_ptr<RouterGroup> group) -> RouterGroup*;
  auto getGroups() const -> std::vector<std::unique_ptr<RouterGroup>> const& { return mGroups; }
  auto getRouter() -> Router& { return mRouter; }
  auto rootGroup() -> RouterGroup*
  {
    assert(mRootGroup == nullptr && "root group already exists");
    auto g = std::make_unique<RouterGroup>("", this, nullptr);
    return mGroups.emplace_back(std::move(g)).get();
  }
  auto newGroup(std::string_view prefix) -> RouterGroup*
  {
    assert(mRootGroup != nullptr);
    auto g = std::make_unique<RouterGroup>(prefix, this, mRootGroup);
    return mGroups.emplace_back(std::move(g)).get();
  }
  auto addRoute(HttpMethod method, std::string_view comp, HandlerType&& handle) -> RouterGroup*
  {
    assert(mRootGroup != nullptr);
    mRouter.addRoute(method, comp, std::move(handle));
    return mRootGroup;
  }
  auto GET(std::string_view comp, HandlerType&& handle) -> RouterGroup*
  {
    return addRoute(HttpMethod::Get, comp, std::move(handle));
  }
  auto POST(std::string_view comp, HandlerType&& handle) -> RouterGroup*
  {
    return addRoute(HttpMethod::Get, comp, std::move(handle));
  }
  auto handle(HttpRequest const& request) -> std::optional<Router::Handler> { return mRouter.handle(request); }

private:
  Router mRouter;
  RouterGroup* mRootGroup {nullptr};
  std::vector<std::unique_ptr<RouterGroup>> mGroups;
};
inline auto Context::runMiddleware() -> Task<bool>
{
  if (groupIndex < groups.size()) {
    auto group = groups[groupIndex];
    groupIndex++;
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
inline auto Context::runAllMiddleware() -> Task<bool>
{
  while (groupIndex < groups.size()) {
    auto res = co_await runMiddleware();
    if (!res) {
      co_return false;
    }
  }
  co_return true;
}
inline auto Context::initGroups(Routers& routers, std::string_view path) -> void
{
  auto& rtGroups = routers.getGroups();
  groups = std::vector<RouterGroup*> {};
  for (auto&& group : rtGroups) {
    auto prefix = group->getPrefix();
    if (path.starts_with(prefix)) {
      groups.push_back(group.get());
    }
  }
  std::sort(groups.begin(), groups.end(),
            [](auto&& a, auto&& b) { return a->getPrefix().size() < b->getPrefix().size(); });
  assert(groups.front()->getPrefix() == "");
}
}; // namespace wf