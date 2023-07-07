cmake_minimum_required(VERSION 3.0.0)

project(cson C)

include_directories(include)

set(SOURCES
  src/cson.c
  src/lexer.c
  src/parser.c
  src/nodelist.c
  include/lexer.h
  include/parser.h
  include/nodelist.h
)

add_executable(cson ${SOURCES})
