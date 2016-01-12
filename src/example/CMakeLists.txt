cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(practica-sistemas-percepcion-mayr)

find_package(OpenCV 2.4 REQUIRED)
find_package(Boost 1.5 COMPONENTS program_options)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

include_directories(../../include)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif()

add_definitions(-DDEBUG_INFORMATION)

add_executable(main main.cpp)
target_link_libraries(main ${Boost_LIBRARIES} ${OpenCV_LIBS})
