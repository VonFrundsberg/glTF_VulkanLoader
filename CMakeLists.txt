cmake_minimum_required(VERSION 3.10)

project(glTF_VulkanLoader VERSION 1.0)

configure_file(glTF_VulkanLoaderConfig.h.in glTF_VulkanLoaderConfig.h)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(FetchContent)
FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG        17254e090e0dc3d5d1aca8efd6303cdbd07dbae1 # version 1.1.0
)
FetchContent_MakeAvailable(rapidjson)

add_library(glTF_VulkanLoaderLib STATIC glTF_Loader.cpp)
#target_include_directories(glTF_VulkanLoaderLib PUBLIC "${CMAKE_BINARY_DIR}/_deps/rapidjson-src/include")
target_include_directories(glTF_VulkanLoaderLib PUBLIC "${rapidjson_SOURCE_DIR}/include")


add_executable(glTF_VulkanLoader Source.cpp)
target_link_libraries(glTF_VulkanLoader PRIVATE glTF_VulkanLoaderLib)
#set_property(TARGET glTF_VulkanLoader PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(glTF_VulkanLoader PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )