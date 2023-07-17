cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_BUILD_TYPE Debug)

project(cson C)

include_directories(include)

set(SOURCES
  src/lexer.c
  src/parser.c
  src/nodelist.c
  src/tokenlist.c
  src/decoders.c
  include/lexer.h
  include/parser.h
  include/nodelist.h
  include/decoders.h
)

add_executable(cson src/cson.c ${SOURCES})
add_executable(decodeTest src/decodeTest.c ${SOURCES})

target_link_libraries(decodeTest PUBLIC m)
target_link_libraries(cson PUBLIC m)

