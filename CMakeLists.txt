cmake_minimum_required(VERSION 2.8)

project(cblibs)

FILE(MAKE_DIRECTORY lib)
FILE(MAKE_DIRECTORY bin/protocol)

aux_source_directory(src/protocol CB_PROTOCOL_LIB_SOURCES)
set(CB_PROTOCOL_EXAMPLE_CHAT_SERVER_SOURCES examples/protocol/chat_server.cpp)
set(CB_PROTOCOL_EXAMPLE_CHAT_CLIENT_SOURCES examples/protocol/chat_client.cpp)

find_package(Boost COMPONENTS system REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)


set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
add_library(cb_protocol STATIC ${CB_PROTOCOL_LIB_SOURCES})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/protocol)

find_package(Threads)


add_executable(chat_server ${CB_PROTOCOL_EXAMPLE_CHAT_SERVER_SOURCES})
add_executable(chat_client ${CB_PROTOCOL_EXAMPLE_CHAT_CLIENT_SOURCES})
target_link_libraries(chat_server ${Boost_LIBRARIES} cb_protocol)
target_link_libraries(chat_client ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} cb_protocol)
