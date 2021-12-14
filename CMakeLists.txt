cmake_minimum_required(VERSION 3.20)
project(sp C)

set(CMAKE_C_STANDARD 99)

add_executable(sp main.c linkedlist.c linkedlist.h structures.h commands.c commands.h filesystem.c filesystem.h inode.c inode.h)
target_link_libraries(sp m)