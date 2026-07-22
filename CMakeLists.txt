cmake_minimum_required(VERSION 3.20)

project(KyraseImageTools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kyrase_image_tools
    src/main.cpp
    src/image.cpp
    src/tester.cpp
    src/reader.cpp
    src/writer.cpp
    src/pipeline_runner.cpp
    src/blurrer.cpp
    src/tests.cpp
    src/image_hash.cpp
 )

target_include_directories(kyrase_image_tools PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)