include_directories("${CMAKE_SOURCE_DIR}")

macro(add_dog_plugin _name _has_resources)
  add_definitions(-DQT_PLUGIN)
  add_definitions(-DQT_SHARED)

  if(${_has_resources})
    qt5_add_resources(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${_name}plugin.qrc)
    add_library(${_name} SHARED ${_name}/${_name}.cpp ${RESOURCES})
  else()
    add_library(${_name} SHARED ${_name}/${_name}.cpp)
  endif()
  target_link_libraries(${_name} doglib)
  qt5_use_modules(${_name} Widgets)
  install(
    TARGETS ${_name}
    RUNTIME DESTINATION lib${LIB_SUFFIX}/dog/plugins/
    LIBRARY DESTINATION lib${LIB_SUFFIX}/dog/plugins/)
endmacro()

add_dog_plugin(backup FALSE)
add_dog_plugin(builddircleaner FALSE)
add_dog_plugin(coredumps FALSE)
add_dog_plugin(modifiedgit FALSE)
# add_dog_plugin(missingfile FALSE)
