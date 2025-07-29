cmake_minimum_required(VERSION 3.14)

project(CubeLoading LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)
find_package(OpenGL REQUIRED)


if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

#Файлы проекта
add_executable(CubeLoading 
main.cpp
mainwindow.cpp
mainwindow.h
ReadWorker.cpp
ReadWorker.h
WriteWorker.cpp
WriteWorker.h
GLWidget.cpp
GLWidget.h
)

##target_link_libraries(QtHelloWorld Qt5::Widgets)
target_link_libraries(CubeLoading
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    ${OPENGL_LIBRARIES}
)