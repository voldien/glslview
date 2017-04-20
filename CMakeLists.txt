CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)
PROJECT(glslview)


EXECUTE_PROCESS(COMMAND git describe
		COMMAND sed -e "s/[^0-9]*//g"
		COMMAND cut -c1-1
	    OUTPUT_VARIABLE _major_version OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _major_version_failed)

EXECUTE_PROCESS(COMMAND git describe
		COMMAND sed -e "s/[^0-9].[^0-9]*//g"
		COMMAND cut -c1-1
	    OUTPUT_VARIABLE _minor_version OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _minor_version_failed)

EXECUTE_PROCESS(COMMAND git describe
		COMMAND sed -e "s/[^0-9].[^0-9].[^0-9]*//g"
		COMMAND cut -c1-1
	    OUTPUT_VARIABLE _revision_version OUTPUT_STRIP_TRAILING_WHITESPACE
	    RESULT_VARIABLE _revision_version_failed)

#	Add version.
ADD_DEFINITIONS(-DGLSLVIEW_MAJOR_VERSION=${_major_version} )
ADD_DEFINITIONS( -DGLSLVIEW_MINOR_VERSION=${_minor_version} )
ADD_DEFINITIONS(-DGLSLVIEW_REVISION_VERSION=${_revision_version} )
SET( VERSION "${_major_version}.${_minor_version}.${_revision_version}")

#	GCC compiler flag options.
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

# Include OpenGL 
INCLUDE(FindOpenGL)
IF(OPENGL_FOUND)

ENDIF()

# Source of the program.
FILE(GLOB core ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c )

# Add
ADD_EXECUTABLE(glslview ${core} )
TARGET_LINK_LIBRARIES(glslview -lSDL2 ${OPENGL_LIBRARIES} -lfreeimage -lvulkan )


# Install program to executable binary directory.
INSTALL(TARGETS glslview DESTINATION bin)
IF( UNIX )
	
	SET( TARGETDIR "${PROJECT_NAME}-${VERSION}")
	ADD_CUSTOM_TARGET(	distribution
				COMMAND mkdir -p ${TARGETDIR}
				COMMAND cp -r src include example CMakeLists.txt README.md *.1 ${TARGETDIR}
				COMMAND tar cf - ${TARGETDIR} | gzip -c > ${TARGETDIR}.tar.gz
				COMMAND rm -r ${TARGETDIR} )

ENDIF()

