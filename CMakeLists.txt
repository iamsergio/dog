cmake_minimum_required(VERSION 2.8.8)

project(dog)
find_package(Qt5Widgets 5.6)
set(CMAKE_AUTOMOC ON)

set(DOG_LIB_SRCS PluginInterface.cpp fileservice.cpp kernel.cpp logger.cpp)

set(DOG_SRCS main.cpp)

add_executable(dog ${DOG_SRCS})
add_library(doglib SHARED ${DOG_LIB_SRCS})

target_compile_definitions(doglib PRIVATE BUILDING_DOG_PLUGIN_INTERFACE)

qt5_use_modules(dog Widgets)
qt5_use_modules(doglib Widgets)
target_link_libraries(dog doglib)

add_subdirectory(plugins)

install (TARGETS dog DESTINATION bin)
install (TARGETS doglib
         RUNTIME DESTINATION bin
         ARCHIVE DESTINATION lib)
