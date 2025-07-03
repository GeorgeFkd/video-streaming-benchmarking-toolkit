cmake_minimum_required(VERSION 3.31)




# set project name from current directory
get_filename_component(BASENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
project(${BASENAME})
find_package(Boost 
	1.87.0
	REQUIRED COMPONENTS math
)

if (not Boost_FOUND) {
	message(FATAL_ERROR "Install boost math library v1.87.0(latest)")
}

set (CMAKE_CXX_STANDARD 11)

aux_source_directory(src SRC_FILES)
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/inc" ${Boost_INCLUDE_DIR})


add_executable (${PROJECT_NAME}test main.cpp)
target_link_libraries (${PROJECT_NAME}test ${PROJECT_NAME}) 
