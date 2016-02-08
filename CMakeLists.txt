cmake_minimum_required(VERSION 2.8)
project(dico-worker)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SOURCE PROTO_HEADERS
	proto/do-task.proto
	proto/handshake.proto
	proto/self-describing-message.proto
	proto/submit-code.proto
	proto/task-result.proto
	proto/task-status.proto)

find_package(LibArchive REQUIRED)
include_directories(${LibArchive_INCLUDE_DIRS})

find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIRS})

aux_source_directory(src WORKER_SOURCE)
include_directories(include)
aux_source_directory(include WORKER_HEADERS)

add_executable(dico_worker ${WORKER_SOURCE} ${WORKER_HEADERS} ${PROTO_SOURCE} ${PROTO_HEADERS})
target_link_libraries(dico_worker Qt5::Core)
target_link_libraries(dico_worker ${PROTOBUF_LIBRARIES})
target_link_libraries(dico_worker ${LibArchive_LIBRARIES})
target_link_libraries(dico_worker ${YAML_CPP_LIBRARIES})
