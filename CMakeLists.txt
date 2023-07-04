cmake_minimum_required(VERSION 3.0.0)

# here we specify that the project is C language only, so the default
# C compiler on the system will be used
project(cson C)

add_executable(cson src/cson.c)
