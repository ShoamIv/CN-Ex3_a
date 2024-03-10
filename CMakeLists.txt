cmake_minimum_required(VERSION 3.27)
project(ex3 C)

set(CMAKE_C_STANDARD 99)

add_executable(TCP_Receiver.c)
add_executable(TCP_Sender TCP_Sender.c)