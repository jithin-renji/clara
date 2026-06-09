cmake_minimum_required(VERSION 3.31.6)

project(clara)

find_package(BISON 3.8.2 REQUIRED)

bison_target(
  parse
  src/parse.y
  parse.c
  DEFINES_FILE parse.h
  COMPILE_FLAGS -Wall
)

add_executable(clara)
target_sources(clara
  PRIVATE
  src/builtins.c src/cmd_ast.c src/env.c src/eval.c src/jobs.c src/main.c
  src/proc.c src/vec.c
  ${BISON_parse_OUTPUTS}
)

target_include_directories(clara PRIVATE src/)

target_link_libraries(clara readline)
