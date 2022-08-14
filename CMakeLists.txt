cmake_minimum_required(VERSION 3.22)
project(Vector_sprint13)

set(CMAKE_CXX_STANDARD 17)

#recent -Wall -pedantic -Wextra -Wstrict-overflow -Werror=vla
#for sanitizer  -fsanitize=address -g -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls
#for profiling -O0 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wstrict-overflow -Werror=vla -fsanitize=address -g -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls")

add_executable(Vector_sprint13 main.cpp test.h test7.h test9.h)