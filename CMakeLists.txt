cmake_minimum_required(VERSION 3.16)
project(mute C)

set(C_STANDARD 99)
set(C_STANDARD_REQUIRED TRUE)
set(BUILD_TYPE "Debug")

add_executable(mute WIN32 "src/mute.c" "res/mute.rc")