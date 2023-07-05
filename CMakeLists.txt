cmake_minimum_required(VERSION 3.0.0)

project(cson C)

include_directories(include)

set(SOURCES
  src/cson.c
  src/lexer.c
  include/lexer.h
)

add_executable(cson ${SOURCES})
