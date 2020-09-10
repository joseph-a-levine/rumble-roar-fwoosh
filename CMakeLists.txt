cmake_minimum_required(VERSION 3.10)
project(final_project_boothsl)

FIND_PACKAGE(OpenMP REQUIRED)
#FIND_PACKAGE(Boost REQUIRED COMPONENTS "tokenizer")

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -Wall")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
SET(DEFAULT_OPTS "-O3")
SET(OPTS "${DEFAULT_OPTS}")

SET(BOOST_ROOT "/usr/include")
SET(BOOST_LIBRARYDIR "/usr/include/boost")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    MESSAGE(STATUS "Apple/macOS system detected")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPTS} -m64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTS} -m64")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    MESSAGE(STATUS "Windows system detected")
    MESSAGE(AUTHOR_WARNING "Compiling for Windows is untested and just a guess, and will require modification!")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPTS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTS}")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    MESSAGE(STATUS "Linux system detected")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPTS} -m64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTS} -m64")
ENDIF()

add_executable(final_project_boothsl main.cpp)