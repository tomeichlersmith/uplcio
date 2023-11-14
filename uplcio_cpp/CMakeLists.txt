# BSD 3-Clause License; see
# https://github.com/scikit-hep/awkward/blob/main/LICENSE

cmake_minimum_required(VERSION 3.15...3.26)

project(
  ${SKBUILD_PROJECT_NAME}
  LANGUAGES CXX
  VERSION ${SKBUILD_PROJECT_VERSION})

message(STATUS "${CMAKE_CURRENT_DIR}")

# Download headers
include(FetchContent)
set(AWKWARD_VERSION "v2.4.3")
FetchContent_Declare(
  awkward-headers
  URL https://github.com/scikit-hep/awkward/releases/download/${AWKWARD_VERSION}/header-only.zip
)
# Instead of using `FetchContent_MakeAvailable(awkward-headers)`, we manually
# load the target so that we can EXCLUDE_FROM_ALL
FetchContent_GetProperties(awkward-headers)
if(NOT awkward-headers_POPULATED)
  FetchContent_Populate(awkward-headers)
  add_subdirectory(${awkward-headers_SOURCE_DIR} ${awkward-headers_BINARY_DIR}
                   EXCLUDE_FROM_ALL)
endif()

# Load interpreter & pybind11
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

option(LCIO_DIR "Path to LCIO C++ installation to build with" $ENV{LCIO_DIR})
find_path(LCIO_INCLUDE_DIR lcio.h ${LCIO_DIR}/include)
find_library(LCIO_LIBRARY lcio ${LCIO_DIR}/lib)
find_library(SIO_LIBRARY sio ${LCIO_DIR}/lib)
set(LCIO_LIBRARIES ${LCIO_LIBRARY} ${SIO_LIBRARY})

# Build demo module
pybind11_add_module(
  uplcio_cpp
  src/uplcio_cpp.cpp
  src/demo.cpp
  src/ReadOnlyFile.cpp
  src/ReadOnlyBranch.cpp
  src/EventHeader.cpp
)
target_link_libraries(uplcio_cpp PRIVATE awkward::layout-builder ${LCIO_LIBRARIES})
target_include_directories(uplcio_cpp PRIVATE src ${LCIO_INCLUDE_DIR})

# Install into wheel
install(TARGETS uplcio_cpp DESTINATION .)
