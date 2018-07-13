cmake_minimum_required(VERSION 2.8.8)

project(dog)
find_package(Qt5Widgets 5.6)
set(CMAKE_AUTOMOC ON)
set(DOG_SRCS fileservice.cpp  kernel.cpp  logger.cpp  main.cpp  PluginInterface.cpp)

add_subdirectory(plugins)

add_executable(dog ${DOG_SRCS})
qt5_use_modules(dog Widgets)

install (TARGETS dog DESTINATION bin)
