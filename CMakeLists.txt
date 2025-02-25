cmake_minimum_required(VERSION 3.22)
project(dhtnet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)
include(GNUInstallDirs)
set (prefix ${CMAKE_INSTALL_PREFIX})
set (exec_prefix "\${prefix}")
set (libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
set (includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")

find_package (PkgConfig REQUIRED)
find_package(msgpack REQUIRED QUIET CONFIG NAMES msgpack msgpack-cxx)
find_package(fmt)
pkg_check_modules (opendht REQUIRED IMPORTED_TARGET opendht>=2.6.0)
pkg_check_modules (pjproject REQUIRED IMPORTED_TARGET libpjproject)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMSGPACK_NO_BOOST -DMSGPACK_DISABLE_LEGACY_NIL -DMSGPACK_DISABLE_LEGACY_CONVERT")

# Sources
list (APPEND dhtnet_SOURCES
    src/connectionmanager.cpp
    src/ice_transport.cpp
    src/multiplexed_socket.cpp
    src/peer_connection.cpp
    src/string_utils.cpp
    src/fileutils.cpp
    src/security/tls_session.cpp
    src/security/certstore.cpp
    src/security/threadloop.cpp
)

list (APPEND dhtnet_HEADERS
    include/connectionmanager.h
    include/multiplexed_socket.h
)

add_library(dhtnet ${dhtnet_SOURCES})
target_link_libraries(dhtnet PUBLIC PkgConfig::opendht fmt::fmt msgpack-cxx)
target_include_directories(dhtnet PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(dhtnet PUBLIC PJ_AUTOCONF=1)
set_target_properties(dhtnet PROPERTIES PUBLIC_HEADER "${dhtnet_HEADERS}")

configure_file(dhtnet.pc.in dhtnet.pc @ONLY)

install(TARGETS dhtnet
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dhtnet
)

# Install targets
install (DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/dhtnet.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if (BUILD_TESTING AND NOT MSVC)
    pkg_search_module(Cppunit REQUIRED IMPORTED_TARGET cppunit)
    add_executable(tests_certstore tests/certstore.cpp)
    target_link_libraries(tests_certstore PRIVATE dhtnet fmt::fmt PkgConfig::Cppunit)
    add_test(NAME tests_certstore COMMAND tests_certstore)

    add_executable(tests_connectionManager tests/connectionManager.cpp)
    target_link_libraries(tests_connectionManager PRIVATE dhtnet fmt::fmt PkgConfig::Cppunit)
    add_test(NAME tests_connectionManager COMMAND tests_connectionManager)

    #add_executable(tests_fileutils tests/testFileutils.cpp)
    #target_link_libraries(tests_fileutils PRIVATE dhtnet fmt::fmt PkgConfig::Cppunit)
    #add_test(NAME tests_fileutils COMMAND tests_fileutils)

    #add_executable(tests_stringutils tests/testString_utils.cpp)
    #target_link_libraries(tests_stringutils PRIVATE dhtnet fmt::fmt PkgConfig::Cppunit)
    #add_test(NAME tests_stringutils COMMAND tests_stringutils)
endif()
