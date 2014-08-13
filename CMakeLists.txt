# Linkbot Firmware Rescue utility
# Barobo, Inc.

cmake_minimum_required(VERSION 2.8.11)

project(LinkbotFirmwareRescue)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

##############################################################################
# Setup

# Setting these two variables to our binary directory will ensure that all
# shared libraries (.so and .dll) will be emitted next to our own executable.
# This allows us to run BaroboBrowser right off the bat, without copying
# libraries around, or messing with path environment variables.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory(../libbarobo libbarobo)

set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH}
    $ENV{QTDIR}lib/cmake/Qt5
   )
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

##############################################################################
# Content

#install(DIRECTORY html DESTINATION ${INSTALL_SHARE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(${LIBBAROBO_INCLUDE_DIRS})
include_directories(${LIBSTKCOMMS_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")
if(WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mwindows")
endif()

##############################################################################
# Targets

set(TARGET LinkbotFirmwareRescue)

set(SOURCES
  src/main.cpp
  src/dialog.cpp
  src/listener.cpp
  )

if(WIN32)
  # Add the application icon 
  list(APPEND SOURCES resource.rc)
endif()

#qt5_wrap_cpp(HEADERS
#  src/dialog.h
#  src/listener.h
#  )

qt5_wrap_ui(UIFILES
  ui/dialog.ui
  )

add_executable(${TARGET} ${SOURCES} ${HEADERS} ${UIFILES})

# FIXME when it becomes feasible to require CMake >= 2.8.11, remove this crap
# search on the internet for "CMake policy CMP0020" to read why
#set(QTMAIN)
#if(WIN32)
#  set(QTMAIN Qt5::WinMain) # imported from Qt5Core
#endif()

target_link_libraries(${TARGET}
  ${LIBBAROBO_LIBRARIES}
  ${LIBSTKCOMMS_LIBRARIES}
  )#${QTMAIN})

qt5_use_modules(${TARGET} Gui Widgets)

if(APPLE)
  # Qt5 headers won't compile without the following...
  set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "-mmacosx-version-min=10.7 -stdlib=libc++")
  set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "-mmacosx-version-min=10.7 -stdlib=libc++")
endif()

##############################################################################
# Installation

if(UNIX)
  set(INSTALL_BIN_DIR bin)
else()
  set(INSTALL_BIN_DIR .)
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION ${INSTALL_BIN_DIR})
