# ------------------------------------------------------------------------------
# Author: Garrett Smith
# File:   CMakeLists.txt
# Date:   04/28/2010
# ------------------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(HELIVIEW_PROJECT)
SET(HELIVIEW_VERSION 0.1)

# default to debug builds unless otherwise specified
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Select build type (None Debug Release RelWithDebInfo MinSizeRel)"
        FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# locate and include project dependencies
FIND_PACKAGE(Boost   COMPONENTS program_options REQUIRED)
FIND_PACKAGE(OGRE    REQUIRED)
FIND_PACKAGE(OpenCV  REQUIRED)
FIND_PACKAGE(OpenGL  REQUIRED)
FIND_PACKAGE(Qt4     COMPONENTS QtCore QtGui QtNetwork QtOpenGL REQUIRED)
FIND_PACKAGE(Qwt5    REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

# put any platform-independent build configuration here
ADD_DEFINITIONS(-D_UNICODE -DUNICODE -D_REENTRANT)

IF(WIN32)
    # put any Windows/MSVC-specific build configuration here
    ADD_DEFINITIONS(-DPLATFORM_WIN_MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)

    # set unicode and pedantic error reporting on windows
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /wd4290")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /D NDEBUG")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /D NDEBUG")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D DEBUG")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D DEBUG")
ENDIF(WIN32)

IF(CMAKE_COMPILER_IS_GNUCXX)
    # put any UNIX/GCC-specific build configuration here
    ADD_DEFINITIONS(-DPLATFORM_UNIX_GCC -DHAVE_STDINT)

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

ADD_SUBDIRECTORY(3rdparty)
ADD_SUBDIRECTORY(src)

