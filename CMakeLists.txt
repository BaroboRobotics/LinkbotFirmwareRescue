# Linkbot Firmware Rescue utility
# Barobo, Inc.

cmake_minimum_required(VERSION 2.8.11)

project(LinkbotFirmwareRescue)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

#
# CONFIG
#

# config variables

# TODO: Use source file properties?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")
if(WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mwindows")
endif()

# To simplify the call to fixup_bundle, make subprojects put their build
# artifacts into our (top-level) tree, not their subtrees.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# requirements

add_subdirectory(../libbarobo libbarobo)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(${LIBBAROBO_INCLUDE_DIRS})
include_directories(${LIBSTKCOMMS_INCLUDE_DIRS})

#
# BUILD
#

set(target LinkbotFirmwareRescue)

# sources

set(sources
  src/main.cpp
  src/dialog.cpp
  src/listener.cpp
  )
if(WIN32)
  list(APPEND sources resource.rc)
endif()

qt5_wrap_ui(uifiles
  ui/dialog.ui
  )

# build targets

add_executable(${target} ${sources} ${uifiles})

target_link_libraries(${target}
  ${LIBBAROBO_LIBRARIES}
  ${LIBSTKCOMMS_LIBRARIES}
  Qt5::Gui
  Qt5::Widgets
  )

#
# STAGE
#

if(WIN32)
  set(dest .)
elseif(APPLE)
  set(dest Contents/MacOS)
else()
  set(dest bin)
endif()

install(TARGETS ${target} RUNTIME DESTINATION ${dest})
