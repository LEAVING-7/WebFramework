#include <gtest/gtest.h>

#include <WebFramework/Http.hpp>
#include <WebFramework/Router.hpp>
class RouterTest : public ::testing::Test {
protected:
  wf::Router router;
  RouterTest()
  {
    router.addRoute(wf::HttpMethod::Get, "/", nullptr);
    router.addRoute(wf::HttpMethod::Get, "/hello/:name", nullptr);
    router.addRoute(wf::HttpMethod::Get, "/hello/b/c", nullptr);
    router.addRoute(wf::HttpMethod::Get, "/hi/:name", nullptr);
    router.addRoute(wf::HttpMethod::Get, "/assets/*filepath", nullptr);
  }
  ~RouterTest() override {}
};

TEST_F(RouterTest, GetRouter)
{
  {
    auto [node, params] = router.getRoute(wf::HttpMethod::Get, "/hello/foobar");
    ASSERT_NE(node, nullptr);
    ASSERT_NE(params, nullptr);
    ASSERT_EQ(node->pattern, "/hello/:name");
    ASSERT_EQ(params->size(), 1);
    ASSERT_EQ(params->at("name"), "foobar");
  }
  {
    auto [node, params] = router.getRoute(wf::HttpMethod::Get, "/hello/b/c");
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->pattern, "/hello/b/c");
    ASSERT_EQ(params->size(), 0);
  }
  {
    auto [node, params] = router.getRoute(wf::HttpMethod::Get, "/assets/css/style.css");
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->pattern, "/assets/*filepath");
    ASSERT_EQ(params->size(), 1);
    ASSERT_EQ(params->at("filepath"), "css/style.css");
  }
  {
    auto [node, params] = router.getRoute(wf::HttpMethod::Get, "/assets/js/app.js");
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->pattern, "/assets/*filepath");
    ASSERT_EQ(params->size(), 1);
    ASSERT_EQ(params->at("filepath"), "js/app.js");
  }
  {
    auto [node, params] = router.getRoute(wf::HttpMethod::Get, "/hello/b/c");
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->pattern, "/hello/b/c");
    ASSERT_EQ(params->size(), 0);
  }
}

class RoutersTest : public ::testing::Test {
protected:
  RoutersTest()
  {
    auto root = routers.rootGroup()->GET("/index", [](wf::Context& ctx) -> Task<bool> {
      ctx.response.status = wf::HttpStatus::Ok;
      ctx.response.version = wf::HttpVersion::Http11;
      ctx.response.reason = "OK";
      ctx.response.body = std::format("Hello");
      co_return true;
    });
    auto v1 = root->newGroup("/v1")
                  ->GET("/",
                        [](wf::Context& ctx) -> Task<bool> {
                          ctx.response.status = wf::HttpStatus::Ok;
                          ctx.response.version = wf::HttpVersion::Http11;
                          ctx.response.reason = "OK";
                          ctx.response.body = std::format("/v1/");
                          co_return true;
                        })
                  ->GET("/hello", [](wf::Context& ctx) -> Task<bool> {
                    auto response = std::make_unique<wf::HttpResponse>();
                    response->status = wf::HttpStatus::Ok;
                    response->version = wf::HttpVersion::Http11;
                    response->reason = "OK";
                    response->body = std::format("/v1/hello");
                    co_return true;
                  });
    auto v2 = root->newGroup("/v2")
                  ->GET("/hello/:name",
                        [](wf::Context& ctx) -> Task<bool> {
                          ctx.response.status = wf::HttpStatus::Ok;
                          ctx.response.version = wf::HttpVersion::Http11;
                          ctx.response.reason = "OK";
                          ctx.response.body = std::format("/v2/hello/{}", ctx.params->at("name"));
                          co_return true;
                        })
                  ->POST("/login/*username", [](wf::Context& ctx) -> Task<bool> {
                    ctx.response.status = wf::HttpStatus::Ok;
                    ctx.response.version = wf::HttpVersion::Http11;
                    ctx.response.reason = "OK";
                    ctx.response.body = std::format("login");
                    co_return true;
                  });
  }
  ~RoutersTest() override {}
  wf::RouterGroup* v1;
  wf::RouterGroup* v2;
  wf::Routers routers;
};

TEST_F(RoutersTest, RouterGroup)
{
  // auto ptr = v1->getParent();
  // while (ptr != nullptr) {
  //   ptr = ptr->getParent();
  // }
}