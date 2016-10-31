CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)
PROJECT(glslview)


execute_process(COMMAND git describe
		COMMAND sed -e "s/[^0-9]*//g"
		COMMAND cut -c1-1
	    OUTPUT_VARIABLE _major_version OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _major_version_failed)

execute_process(COMMAND git describe
		COMMAND sed -e "s/[^0-9].[^0-9]*//g"
		COMMAND cut -c1-1
	    OUTPUT_VARIABLE _minor_version OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _minor_version_failed)

execute_process(COMMAND git describe
		COMMAND sed -e "s/[^0-9].[^0-9].[^0-9]*//g"
		COMMAND cut -c1-1
	    OUTPUT_VARIABLE _revision_version OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _revision_version_failed)

#	Add version.
ADD_DEFINITIONS(-DGLSLVIEW_MAJOR_VERSION=${_major_version} )
ADD_DEFINITIONS( -DGLSLVIEW_MINOR_VERSION=${_minor_version} )
ADD_DEFINITIONS(-DGLSLVIEW_REVISION_VERSION=${_revision_version} )

#	GCC Compiler.
if((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND NOT CMAKE_COMPILER_IS_MINGW)

	ADD_DEFINITIONS( -Wall -w -fpermissive   -Wfatal-errors )

	IF (CMAKE_BUILD_TYPE STREQUAL "Release")
		MESSAGE(STATUS "Compile for release.")
		ADD_DEFINITIONS(-DNDEBUG)
		ADD_DEFINITIONS(-O3)
		SET(CMAKE_RELEASE TRUE)
		SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
	ELSE()
		MESSAGE(STATUS "Compile for debug.")
		ADD_DEFINITIONS(-D_DEBUG)
		ADD_DEFINITIONS(-g3 -g)
		SET(CMAKE_DEBUG TRUE)
	ENDIF()
endif()
INCLUDE_DIRECTORIES(./include)

INCLUDE(FindOpenGL)
IF(OPENGL_FOUND)

ENDIF()


FILE(GLOB core ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c )
ADD_EXECUTABLE(glslview ${core} )

TARGET_LINK_LIBRARIES(glslview -lSDL2 -lEngineEx ${OPENGL_LIBRARIES} -lfreeimage -lvulkan)

INSTALL(TARGETS glslview DESTINATION bin)
IF( UNIX )
	EXECUTE_PROCESS(OUTPUT_FILE glslview.1.gz
			COMMAND gzip -ck glslview.1)
	INSTALL(FILES glslview.1.gz DESTINATION /usr/share/man/man1/)
	EXECUTE_PROCESS(COMMAND mandb)
ENDIF()
#INSTALL(TARGETS glslview.1 DESTINATION man)






