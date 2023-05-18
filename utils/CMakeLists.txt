file(GLOB WEBFRAMEWORK_UTILS_SOURCE
    "./*.cpp"
)
add_library(WebFrameworkUtils OBJECT ${WEBFRAMEWORK_UTILS_SOURCE})
target_include_directories(WebFrameworkUtils
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)