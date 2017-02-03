# set for test in other cmake files
set(PLATFORM_QT ON)

if(EXISTS "/etc/debian_version")
      include(${CMAKE_ROOT}/Modules/MultiArchCross.cmake OPTIONAL
RESULT_VARIABLE _INCLUDED_MULTIARCH_TOOLCHAIN_FILE)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC")

# global compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")

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

# compile definitions (adds -DPLATFORM_QT)
set(CORE_COMPILE_DEFS PLATFORM_QT)

if (USE_EXTERNAL_LIBS)
  include(${EXTERNAL_LIBS_DIR}/core-dependencies.cmake)
else()
  add_subdirectory(${PROJECT_SOURCE_DIR}/external)
endif()

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
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/tangramquick_plugin.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/tangramquick.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/qtangramgesturearea.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/qtangrammap.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/qtangrammapcontroller.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/qtangramgeometry.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/qtangrampolyline.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/qtangrampoint.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/platform_gl.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/contentdownloader.cpp
  ${CMAKE_SOURCE_DIR}/qt/tangramquick/platform_qt.cpp)

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
	COMMAND cp ${PROJECT_SOURCE_DIR}/qt/tangramquick/qmldir ${PROJECT_BINARY_DIR}/bin/com/mapzen/tangram)


if(APPLICATION)

  set(EXECUTABLE_NAME "tangram")

  # add sources and include headers
  find_sources_and_include_directories(
    ${PROJECT_SOURCE_DIR}/qt/tangramquick-demo/*.h
    ${PROJECT_SOURCE_DIR}/qt/tangramquick-demo/*.cpp)

  add_executable(${EXECUTABLE_NAME} ${SOURCES}
	  ${PROJECT_SOURCE_DIR}/qt/tangramquick-demo/qml.qrc)
  target_link_libraries(${EXECUTABLE_NAME} Qt5::Core Qt5::Qml Qt5::Quick)

  add_resources(${EXECUTABLE_NAME} "${PROJECT_SOURCE_DIR}/scenes")

endif()

