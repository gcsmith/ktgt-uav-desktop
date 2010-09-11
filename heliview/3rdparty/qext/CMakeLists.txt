# ------------------------------------------------------------------------------
# Author: Garrett Smith
# File:   qext/CMakeLists.txt
# Date:   04/28/2010
# ------------------------------------------------------------------------------

PROJECT(qext_project)

INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${QT_QTCORE_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${QT_QTGUI_INCLUDE_DIR})
INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS(-DQEXTSERIALPORT_LIB)

IF(WIN32)
	SET(platform_deps
		src/win_qextserialport.cpp
		src/qextserialenumerator_win.cpp)
ELSE(WIN32)
	SET(platform_deps
        	src/posix_qextserialport.cpp
        	src/qextserialenumerator_unix.cpp)
ENDIF(WIN32)

SET(qext_src_cpp
	${platform_deps}
        src/qextserialport.cpp)

SET(qext_src_moc
        src/qextserialport.h
        src/qextserialenumerator.h)

QT4_AUTOMOC(${qext_src_moc})
QT4_WRAP_CPP(qext_moc ${qext_src_moc})

ADD_LIBRARY(qext ${qext_src_cpp} ${qext_moc})

