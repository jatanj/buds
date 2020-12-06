cmake_minimum_required(VERSION 3.5)
project(buds+)

option(REQUIRE_STATIC "Require static libraries" ON)
option(DEBUG_ASAN "Use address sanitizer" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unknown-pragmas")

add_definitions(-D_REENTRANT)

set(ASAN_FLAGS "-g3 -fsanitize=address -fsanitize=leak -fsanitize=undefined -fno-omit-frame-pointer")
if (DEBUG_ASAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ASAN_FLAGS}")
endif()

if (REQUIRE_STATIC)
  list(REMOVE_ITEM CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 ".a")
endif()

find_file(CONAN_BUILD_INFO_CMAKE conanbuildinfo.cmake PATHS
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/build")
include(${CONAN_BUILD_INFO_CMAKE})
conan_basic_setup()

add_executable(buds+
  src/buds+/main.cpp
  src/buds+/bluetooth_client.h
  src/buds+/bluetooth_client.cpp
  src/buds+/buds_client.h
  src/buds+/buds_client.cpp
  src/buds+/message_builder.h
  src/buds+/message_parser.h
  src/buds+/message_parser.cpp
  src/buds+/output.h
  src/buds+/argos_output.h
  src/buds+/argos_output.cpp
  src/buds+/config.h
  src/buds+/config.cpp
  src/buds+/cli.h
  src/buds+/cli.cpp
  src/buds+/log.h
  src/buds+/log.cpp
  src/buds+/util.h
  src/buds+/util.cpp
  src/buds+/galaxy-buds-rfcomm-crc16-ccitt.h
  src/buds+/galaxy-buds-rfcomm-crc16-ccitt.cpp)
target_link_libraries(buds+
  PRIVATE bluetooth
  PRIVATE ${CONAN_LIBS})
set_target_properties(buds+ PROPERTIES
  OUTPUT_NAME "buds+"
  LINKER_LANGUAGE CXX)

install(TARGETS buds+ DESTINATION bin)
