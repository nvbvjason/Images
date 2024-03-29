cmake_minimum_required(VERSION 3.28)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled main.cpp
                        png_helpers.cpp
                        png_helpers.h
                        Bmp.cpp
                        Bmp.h
                        Image3x8.h
                        Image3x8.cpp
                        ReadPNG.cpp
                        read_file.cpp
                        read_file.h
                        test.cpp
                        test.h)
