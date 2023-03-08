set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/plugins")

set(CMAKE_BUILD_RPATH ./)
file(COPY ${CMAKE_SOURCE_DIR}/plugins/opcua/opcuaserver.json DESTINATION ${CMAKE_BINARY_DIR}/plugins/schema/)

set(PLUGIN_NAME plugin-opcua)
set(PLUGIN_SOURCES opcua_plugin.c opcua_node_manager.c opcua_handle.c)
add_library(${PLUGIN_NAME} SHARED)
target_include_directories(${PLUGIN_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/neuron)
target_sources(${PLUGIN_NAME} PRIVATE ${PLUGIN_SOURCES})
target_link_libraries(${PLUGIN_NAME} neuron-base open62541 pthread -flto=auto)