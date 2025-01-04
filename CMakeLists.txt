cmake_minimum_required(VERSION 3.22)

project(vtutil_test VERSION 0.0.0)

# reference: 
# https://qiita.com/yohm/items/db6597ffb87fa5480cea
# https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md
# https://qiita.com/ousttrue/items/4fa7a786a6c51e9f11f0

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# juce_add_以外の場合に使用する。今回は使わない
# set(JUCE_MODULES_ONLY ON)

# Declare&MakeAvailableの場合、
# _depsディレクトリに落としてきてadd_subdirectory()し、cmakeの管理に追加する
include(FetchContent)

######################################
# googletest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2)

# 親プロジェクトのコンパイラ・リンカ設定を上書きするのを防ぐ(?)（Windowsのみ）
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
message(STATUS "installed googletest")

######################################
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 7.0.9)
FetchContent_MakeAvailable(JUCE)
message(STATUS "installed JUCE")

######################################
enable_testing()

juce_add_console_app(DummyApp PRODUCT_NAME "Test Runner")
juce_add_module(${CMAKE_CURRENT_LIST_DIR})

file(
  GLOB_RECURSE 
  TEST_SOURCES 
  # CONFIGURE_DEPENDS
  Tests/*.cpp)

target_sources(
  DummyApp
  PUBLIC ${TEST_SOURCES})

target_compile_definitions(DummyApp PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0)

target_link_libraries(
  DummyApp 
PRIVATE
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
  juce::juce_core
  juce::juce_events
  juce::juce_data_structures
  vtutil
  gtest)


include(GoogleTest)
gtest_discover_tests(DummyApp)
