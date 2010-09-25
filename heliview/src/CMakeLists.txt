# ------------------------------------------------------------------------------
# Author: Garrett Smith
# File:   src/CMakeLists.txt
# Date:   04/28/2010
# ------------------------------------------------------------------------------

PROJECT(heliview_src_project)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${HELIVIEW_PROJECT_SOURCE_DIR}/3rdparty/qext/src)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Qwt5_INCLUDE_DIR})
INCLUDE(${QT_USE_FILE})

SET(heliview_deps
        qext
        ${OPENGL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${QT_LIBRARIES}
        ${Qwt5_Qt4_LIBRARY}
        ${OGRE_LIBRARIES})

IF(WIN32)
    SET(heliview_plat_cpp
            WindowsGamepad.cpp)
    SET(heliview_plat_moc
            WindowsGamepad.h)
    SET(heliview_plat_flag WIN32)
ELSE(WIN32)
    SET(heliview_plat_cpp
            LinuxGamepad.cpp)
    SET(heliview_plat_moc
            LinuxGamepad.h)
    SET(heliview_plat_flag )
ENDIF(WIN32)

SET(heliview_cpp
        ApplicationFrame.cpp
        ControllerView.cpp
        DeviceController.cpp
        HeliView.cpp
        LineGraph.cpp
        NetworkDeviceController.cpp
        SerialDeviceController.cpp
        SimulatedDeviceController.cpp
        VirtualView.cpp
        VideoView.cpp
        ${heliview_plat_cpp})

SET(heliview_src_moc
        ApplicationFrame.h
        ControllerView.h
        DeviceController.h
        Gamepad.h
        LineGraph.h
        NetworkDeviceController.h
        SerialDeviceController.h
        SimulatedDeviceController.h
        VirtualView.h
        VideoView.h
        ${heliview_plat_moc})

SET(heliview_src_qrc
        HeliView.qrc)

SET(heliview_src_ui
        ApplicationFrame.ui)

QT4_AUTOMOC(${heliview_src_moc})
QT4_WRAP_CPP(heliview_moc ${heliview_src_moc})
QT4_WRAP_UI(heliview_ui ${heliview_src_ui})
QT4_ADD_RESOURCES(heliview_qrc ${heliview_src_qrc})

ADD_EXECUTABLE(heliview ${heliview_plat_flag} ${heliview_cpp}
               ${heliview_qrc} ${heliview_moc} ${heliview_ui})

TARGET_LINK_LIBRARIES(heliview ${heliview_deps})

