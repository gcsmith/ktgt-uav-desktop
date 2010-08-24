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

SET(qext_src_cpp
        src/qextserialport.cpp
        src/posix_qextserialport.cpp
        src/qextserialenumerator_unix.cpp)

SET(qext_src_moc
        src/qextserialport.h
        src/qextserialenumerator.h)

QT4_AUTOMOC(${qext_src_moc})
QT4_WRAP_CPP(qext_moc ${qext_src_moc})

ADD_LIBRARY(qext ${qext_src_cpp} ${qext_moc})

