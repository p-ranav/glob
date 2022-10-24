cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  Glob
  VERSION 1.0
  LANGUAGES CXX
)

# ---- Options ----
option(GLOB_USE_GHC_FILESYSTEM "Use ghc::filesystem instead of std::filesystem" OFF)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage(
  NAME PackageProject.cmake
  GITHUB_REPOSITORY TheLartians/PackageProject.cmake
  VERSION 1.3
)

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(Glob INTERFACE) set_target_properties(Glob PROPERTIES
# INTERFACE_COMPILE_FEATURES cxx_std_17)

add_library(Glob ${headers} ${sources})
SET_TARGET_PROPERTIES(Glob PROPERTIES OUTPUT_NAME glob)
set_target_properties(Glob PROPERTIES CXX_STANDARD 17)

if ( GLOB_USE_GHC_FILESYSTEM )
  # Switch to ghc::filesystem.
  target_link_libraries(Glob PRIVATE ghcFilesystem::ghc_filesystem)
  target_compile_definitions(Glob PUBLIC GLOB_USE_GHC_FILESYSTEM)
endif()

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(Glob PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")

# Link dependencies (if required) target_link_libraries(Glob PUBLIC cxxopts)

target_include_directories(
  Glob PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                 $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  DEPENDENCIES ""
)
