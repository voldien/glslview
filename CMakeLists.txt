cmake_minimum_required(VERSION 2.8.0)
PROJECT(glslview)



EXECUTE_PROCESS(COMMAND " git describe | sed -e 's/[^0-9]*//g' | cut -c1-1 "
	    OUTPUT_VARIABLE _vd_major OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _major_version_failed)

EXECUTE_PROCESS(COMMAND  " git describe | sed -e 's/[^0-9].[^0-9]*//g' | cut -c1-1 "
	    OUTPUT_VARIABLE _vd_minor OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _minor_version_failed)

EXECUTE_PROCESS(COMMAND "git describe | sed -e 's/[^0-9].[^0-9].[^0-9]*//g' | cut -c1-2"
	    OUTPUT_VARIABLE _vd_revision OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _revision_version_failed)





if((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND NOT CMAKE_COMPILER_IS_MINGW)
  	if (BUILD_SHARED_LIBS AND CMAKE_SIZEOF_VOID_P EQUAL 8) # -fPIC is only required
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	endif()

	ADD_DEFiNITIONS( -Wall)
	ADD_DEFiNITIONS(-w)

	ADD_DEFINITIONS( -fpermissive )

	ADD_DEFINITIONS( -Wall -w -fpermissive   -Wfatal-errors )

	IF (CMAKE_BUILD_TYPE STREQUAL "Release")
		MESSAGE(STATUS "Compile for release.")
		ADD_DEFINITIONS(-DNDEBUG)
		ADD_DEFINITIONS(-O3)
		SET(CMAKE_RELEASE TRUE)
	ELSE()
		MESSAGE(STATUS "Compile for debug.")
		ADD_DEFINITIONS(-D_DEBUG)
		ADD_DEFINITIONS(-g3 -g)
		SET(CMAKE_DEBUG TRUE)
	ENDIF()

	#INCLUDE(FindSSE)
	#IF(SSE4_2_FOUND)
	#	MESSAGE(STATUS "sse42")
	#	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse42")
	#ELSEIF(SSE4_1_FOUND)
	#	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse41")
	#ENDIF()
endif()




FILE(GLOB core ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c )
ADD_EXECUTABLE(glslview ${core} )
TARGET_LINK_LIBRARIES(glslview -lEngineEx -lGL)



INSTALL(TARGETS glslview DESTINATION bin)


