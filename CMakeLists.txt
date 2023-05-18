cmake_minimum_required(VERSION 3.22)
project(WebFramework VERSION 0.1.0 LANGUAGES CXX)

# set(CMAKE_CXX_COMPILER "/usr/bin/gcc-13")
# set(CMAKE_CXX_FLAGS "-Wall -lm -lstdc++")

set(CMAKE_CXX_STANDARD 20)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "-fcolor-diagnostics ${CMAKE_CXX_FLAGS}")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always ${CMAKE_CXX_FLAGS}")
endif()

include(FetchContent)
macro(AddExternal target url git_tag)
  FetchContent_Declare(
    ${target}
    GIT_REPOSITORY https://github.com/${url}
    GIT_TAG ${git_tag}
  )
  FetchContent_MakeAvailable(${target})
endmacro()

file(GLOB SOURCES 
    "lib/*.cpp"
)

add_library(WebFramework STATIC)
target_sources(WebFramework PRIVATE ${SOURCES})
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
add_subdirectory(tests)
add_subdirectory(examples)
