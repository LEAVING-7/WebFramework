cmake_minimum_required(VERSION 3.22)
project(WebFramework VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(GLOBAL_GITHUB_ADDRESS "https://ghproxy.com/https://github.com" CACHE STRING "Global github address")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "-Wall -fcolor-diagnostics ${CMAKE_CXX_FLAGS}")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "-lstdc++ -g -lm -Wall -fdiagnostics-color=always ${CMAKE_CXX_FLAGS}")
endif()

include(FetchContent)
macro(AddExternal target url git_tag)
  if(NOT DEFINED GLOBAL_GITHUB_ADDRESS)
    set(GLOBAL_GITHUB_ADDRESS "https://github.com")
  endif()
  FetchContent_Declare(
    ${target}
    GIT_REPOSITORY "${GLOBAL_GITHUB_ADDRESS}/${url}"
    GIT_TAG ${git_tag}
  )
FetchContent_MakeAvailable(${target})
endmacro()

file(GLOB WebFramework_SOURCES 
    "lib/*.cpp"
)

add_library(WebFramework STATIC)
target_sources(WebFramework PRIVATE ${WebFramework_SOURCES})
target_include_directories(WebFramework
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)
add_subdirectory(utils)
target_link_libraries(WebFramework PUBLIC AsyncIO WebFrameworkUtils)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "-fsanitize=address ${CMAKE_CXX_FLAGS}") 
  target_compile_definitions(WebFramework PRIVATE DEBUG)
endif()
add_subdirectory(submodule)

option(WebFramework_BUILD_TESTS "Build tests" ON)
option(WebFramework_BUILD_EXAMPLES "Build examples" ON)

if(${WebFramework_BUILD_TESTS})
  add_subdirectory(tests)
endif()
if(${WebFramework_BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()
