cmake_minimum_required(VERSION 3.2)
project(dnsproxy C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

file(GLOB SRC_FILES "src/*.c")


include(FetchContent)

FetchContent_Declare(
  cyaml
  GIT_REPOSITORY https://github.com/tlsa/libcyaml
  GIT_TAG v1.4.2
)

FetchContent_MakeAvailable(cyaml)


set(CYAML_STATIC_LIB ${cyaml_SOURCE_DIR}/build/release/libcyaml.a)

add_custom_command(
  OUTPUT ${CYAML_STATIC_LIB}
  COMMAND make
  WORKING_DIRECTORY ${cyaml_SOURCE_DIR}
  COMMENT "Building libcyaml"
)

add_custom_target(build_cyaml DEPENDS ${CYAML_STATIC_LIB})

FetchContent_Declare(
  c_thread_pool
  GIT_REPOSITORY https://github.com/Pithikos/C-Thread-Pool.git
  GIT_TAG master
)

FetchContent_MakeAvailable(c_thread_pool)

add_executable(${PROJECT_NAME} main.c ${SRC_FILES} ${c_thread_pool_SOURCE_DIR}/thpool.c)

add_dependencies(${PROJECT_NAME} build_cyaml)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third_party
  ${cyaml_SOURCE_DIR}/include/cyaml
  ${c_thread_pool_SOURCE_DIR}/
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  ${CYAML_STATIC_LIB}
  yaml
)
