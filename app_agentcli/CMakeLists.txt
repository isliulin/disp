#/usr/bin/cmake

cmake_minimum_required (VERSION 0.90)


#project name
MESSAGE("cmake project...")
PROJECT(agentcli)

#head file path

INCLUDE_DIRECTORIES(
    /usr/include/boost
	.
	../public
	../comm_supp
)

MESSAGE(PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR})
MESSAGE(PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR})


#source directory
AUX_SOURCE_DIRECTORY(.   MAIN_SRCS)

SET(DIR_SRCS ${MAIN_SRCS} ../public/MessageQueue.cpp ../public/CSemaphore.cpp ../public/mktdata.pb.cpp ../public/d31data.pb.cpp ../public/wwtiny.cpp ../comm_supp/callsupp.cpp)

MESSAGE(DIR_SRCS=${DIR_SRCS})

#set environment var
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../lib)
MESSAGE(EXECUTABLE_OUTPUT_PATH=${EXECUTABLE_OUTPUT_PATH})
MESSAGE(LIBRARY_OUTPUT_PATH=${LIBRARY_OUTPUT_PATH})

SET(TARGET_BIN agentcli)

LINK_DIRECTORIES(/usr/local/lib /usr/local/lib64  /usr/lib/)

#add executable file
ADD_EXECUTABLE(${TARGET_BIN} ${DIR_SRCS})


#add link library
SET(LIBRARIES 
	pthread 
	boost_system 
	boost_thread 
	protobuf 
	stdc++ 
	rt
	)

MESSAGE(LIBRARIES=${LIBRARIES})
#MESSAGE(LINK_DIRECTORIES=${LINK_DIRECTORIES})  

ADD_DEFINITIONS(
        -O0 -g -W -Wall -std=c++11
        -Wunused-variable -Wunused-parameter -Wunused-function -Wunused
        -Wno-deprecated -Woverloaded-virtual -Wwrite-strings
        -DLINUX
        -DDEF_LINUX 
        -DSYSTEM_V
)

TARGET_LINK_LIBRARIES(${TARGET_BIN} ${LIBRARIES})


