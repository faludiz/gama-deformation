cmake_minimum_required(VERSION 3.5)

project(gen-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES gen-deformation-data.cpp point.h obs.h obs.cpp
  transformation.h transformation.cpp)

add_executable(gen-deformation-data-0 ${SOURCE_FILES})
add_executable(gen-deformation-data-1 ${SOURCE_FILES})
add_executable(gen-deformation-data-2 ${SOURCE_FILES})

target_compile_definitions(gen-deformation-data-1 PRIVATE -DEPOCH_1)
target_compile_definitions(gen-deformation-data-2 PRIVATE -DEPOCH_2)
