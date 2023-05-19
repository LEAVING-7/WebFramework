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

Task<int> foo() { co_return 1; }

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