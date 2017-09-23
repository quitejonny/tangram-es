if(EXISTS "/etc/debian_version")
      include(${CMAKE_ROOT}/Modules/MultiArchCross.cmake OPTIONAL
RESULT_VARIABLE _INCLUDED_MULTIARCH_TOOLCHAIN_FILE)
endif()

# global compile options
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-zero-variadic-macro-arguments")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -lc++ -lc++abi")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)
  string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
  list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
  list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)

  message(STATUS "Using gcc ${GCC_VERSION}")
  if (GCC_VERSION VERSION_GREATER 5.1)
    message(STATUS "USE CXX11_ABI")
    add_definitions("-D_GLIBCXX_USE_CXX11_ABI=1")
  endif()
endif()

check_unsupported_compiler_version()

add_definitions(-DTANGRAM_QT)

# load core library
add_subdirectory(${PROJECT_SOURCE_DIR}/core)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find the Qt libraries
find_package(Qt5Core)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5Location)

set(PLUGIN_NAME tangramquick) # in order to have libtangramquick.so

add_library(${PLUGIN_NAME} SHARED
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/tangramquick_plugin.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/tangramquick.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/qtangramgesturearea.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/qtangramgeometry.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/qtangrampolyline.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/qtangrampoint.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/platform_gl.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/contentdownloader.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/qtangrammarkermanager.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/tangramquickrenderer.cpp
  ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/platform_qt.cpp)

target_link_libraries(${PLUGIN_NAME}
  PUBLIC
  Qt5::Core Qt5::Qml Qt5::Quick Qt5::Location Qt5::Network
  ${CORE_LIBRARY}
   -ldl
   -pthread
   -lgcc_s
   -lgcc
   )

set_target_properties(${PLUGIN_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/com/mapzen/tangram)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(${PLUGIN_NAME}-qmldir ALL
  COMMAND cp ${PROJECT_SOURCE_DIR}/platforms/qt/tangram/qmldir ${PROJECT_BINARY_DIR}/bin/com/mapzen/tangram)


if(TANGRAM_APPLICATION)

  set(EXECUTABLE_NAME "tangram")

  # add sources and include headers
  find_sources_and_include_directories(
    ${PROJECT_SOURCE_DIR}/platforms/qt/demo/*.h
    ${PROJECT_SOURCE_DIR}/platforms/qt/demo/*.cpp)

  add_executable(${EXECUTABLE_NAME}
    ${SOURCES}
    ${PROJECT_SOURCE_DIR}/platforms/qt/demo/qml.qrc)
  target_link_libraries(${EXECUTABLE_NAME} Qt5::Core Qt5::Qml Qt5::Quick)

  add_resources(${EXECUTABLE_NAME} "${PROJECT_SOURCE_DIR}/scenes")

endif()

