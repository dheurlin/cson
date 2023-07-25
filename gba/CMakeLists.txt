cmake_minimum_required(VERSION 3.1)
project(cson C)
include_directories(../include)

set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/lexer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/parser.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/nodelist.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/tokenlist.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/decoders.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/stringbuilder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/lexer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/nodelist.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/decoders.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/stringbuilder.h
)

add_library(cson STATIC ${SOURCES})

target_compile_options(cson PRIVATE
  -mabi=aapcs -march=armv4t -mcpu=arm7tdmi -mthumb -ffunction-sections -fdata-sections -Wall -Wextra -Wno-unused-parameter>
)

target_link_libraries(cson PUBLIC m)

