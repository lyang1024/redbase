# Author: Payas
# E-mail: payas.rajan@email.ucr.edu
# 2017: Migrated from vanilla Make to CMake. 
# 	TODO: Check windows builds

cmake_minimum_required (VERSION 3.5.2)
project (Redbase)

# Set compiler parameters
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Payas: Required to print statistics from the PF Layer.  DONOT TOUCH.
add_definitions(-DPF_STATS)

# Include directories
include_directories(include)

# Source files grouped by functions
set(PF_SOURCES  
		src/pf_buffermgr.cc 
		src/pf_error.cc
		src/pf_filehandle.cc 
		src/pf_pagehandle.cc 
		src/pf_hashtable.cc
		src/pf_manager.cc
        src/pf_statistics.cc
		src/statistics.cc
	)

set(RM_SOURCES
		src/rm_manager.cc 
		src/rm_filehandle.cc 
		src/rm_record.cc 
        src/rm_filescan.cc 
		src/rm_error.cc 
		src/rm_rid.cc
	)

set(SM_SOURCES
		src/sm_manager.cc
		src/printer.cc
		src/sm_error.cc
		src/sm_attriterator.cc
	)

set(QL_SOURCES
		src/ql_error.cc 
		src/ql_manager.cc 
		src/ql_node.cc 
		src/ql_nodeproj.cc 
		src/ql_noderel.cc
		src/ql_nodejoin.cc 
		src/ql_nodesel.cc 
		src/qo_manager.cc
	)

set(IX_SOURCES
		src/ix_manager.cc
		src/ix_indexhandle.cc
		src/ix_indexscan.cc
		src/ix_error.cc
	)
set (PARSER_SOURCES
		src/nodes.cc
		src/interp.cc
	)

# Handle Flex/Bison for Query Parser
FIND_PACKAGE(BISON REQUIRED)
BISON_TARGET(MyParser src/parse.y ${CMAKE_CURRENT_BINARY_DIR}/parse.cpp)

FIND_PACKAGE(FLEX REQUIRED)
FLEX_TARGET(MyScanner src/scan.l ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp)

ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

# Create libraries
add_library(pf STATIC "${PF_SOURCES}")
add_library(rm STATIC "${RM_SOURCES}")
add_library(sm STATIC "${SM_SOURCES}")
add_library(ql STATIC "${QL_SOURCES}")
add_library(ix STATIC "${IX_SOURCES}")
add_library(parser STATIC "${PARSER_SOURCES}" ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS})

# Link the Library calls
target_link_libraries(rm pf sm ql ix parser) 
target_link_libraries(sm rm ix pf parser)
target_link_libraries(ql sm)
target_link_libraries(ix rm)
target_link_libraries(parser pf)


# Compile shells
add_executable(redbase src/redbase.cc)
add_executable(dbcreate src/dbcreate.cc)
add_executable(dbdestroy src/dbdestroy.cc)

# Link shells with the libraries
target_link_libraries(redbase pf rm sm ql ix parser)
target_link_libraries(dbcreate pf rm sm ql ix parser)
target_link_libraries(dbdestroy pf rm sm ql ix parser)

