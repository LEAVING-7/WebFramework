enable_testing()
AddExternal(gtest; google/googletest; v1.13.0)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_executable(test_Router test_Router.cpp)
target_link_libraries(test_Router PUBLIC gtest_main WebFramework)