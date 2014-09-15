# Linkbot Firmware Rescue utility
# Barobo, Inc.

cmake_minimum_required(VERSION 2.8.11)

project(LinkbotFirmwareRescue)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# This project has one executable:

set(target LinkbotFirmwareRescue)

set(sources
  src/main.cpp
  src/dialog.cpp
  src/listener.cpp
  )
if(WIN32)
  list(APPEND sources appIcon.rc)
endif()

# We get to do this here so qt5_wrap_ui works
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

qt5_wrap_ui(uifiles ui/dialog.ui)

add_executable(${target} ${sources} ${uifiles})

#
# CONFIG
#

# target properties

set_target_properties(${target} PROPERTIES
  COMPILE_FLAGS "-std=gnu++11"
  COMPILE_FLAGS_RELEASE "-DQT_NO_DEBUG_OUTPUT"
  AUTOMOC ON
)
if(WIN32)
  set_target_properties(${target} PROPERTIES
    LINK_FLAGS_RELEASE "-mwindows"
  )
endif()

# To simplify the call to fixup_bundle, make subprojects put their build
# artifacts into our (top-level) tree, not their subtrees.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# requirements

add_subdirectory(../libbarobo barobo)

include_directories($<TARGET_PROPERTY:barobo,INCLUDE_DIRECTORIES>)

# Linker requirements

target_link_libraries(${target}
  barobo
  stkcomms
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
