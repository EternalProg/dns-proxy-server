cmake_minimum_required(VERSION 3.2)
project(dnsproxy)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)


file(GLOB SRC_FILES "src/*.c")
add_executable(${PROJECT_NAME} main.c ${SRC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

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
add_dependencies(${PROJECT_NAME} build_cyaml)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third_party
  ${cyaml_SOURCE_DIR}/include/cyaml
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CYAML_STATIC_LIB} yaml)

