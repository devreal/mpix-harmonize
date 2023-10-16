cmake_minimum_required(VERSION 3.22)
project(mpix-harmonize VERSION 0.1 LANGUAGES C DESCRIPTION "Implementation of MPIX_Harmonize")

include(GNUInstallDirs)
include(FetchContent)
include(CMakePackageConfigHelpers)

find_package(MPI REQUIRED)

set(MPITS_CLOCK "mpi_wtime" CACHE STRING "time source in libmpits")
set_property(CACHE MPITS_CLOCK PROPERTY STRINGS mpi_wtime monotonic realtime)
message(STATUS "mpits clock: ${MPITS_CLOCK}")

if(${MPITS_CLOCK} STREQUAL "mpi_wtime")
    # default, do nothing
elseif(${MPITS_CLOCK} STREQUAL "monotonic")
    set(ENABLE_GETTIME_MONOTONIC ON)
elseif(${MPITS_CLOCK} STREQUAL "realtime")
    set(ENABLE_GETTIME_REALTIME ON)
else()
    message(FATAL_ERROR "Invalid value for MPITS_CLOCK: ${MPITS_CLOCK}. Allowed values are:  mpi_wtime, monotonic, or realtime.")
endif()


# Configure the installation paths
set(MPIX_HARMONIZE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
set(MPIX_HARMONIZE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(MPIX_HARMONIZE_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
set(MPIX_HARMONIZE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBEXECDIR})
set(MPIX_HARMONIZE_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR})
set(MPIX_HARMONIZE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATADIR}/cmake/mpix-harmonize)


FetchContent_Declare(
        MPITS
        GIT_REPOSITORY git@github.com:hunsa/mpi-time-sync.git
        GIT_TAG main)

FetchContent_MakeAvailable(MPITS)
FetchContent_GetProperties(
        MPITS
        SOURCE_DIR MPITS_SOURCE_DIR
        BINARY_DIR MPITS_BINARY_DIR
        INCLUDE_DIR MPITS_INCLUDE_DIR
        INSTALL_DIR MPITS_INSTALL_DIR )

export(EXPORT mpits-targets FILE "${PROJECT_BINARY_DIR}/mpits-targets.cmake")
set(MPITS_CONFIG ${CMAKE_INSTALL_PREFIX}/share/cmake/mpits/mpits-config.cmake)

if (NOT TARGET MPITS::mpits)
    message(FATAL "FAILED TO FIND MPITS")
endif()

add_library(mpix-harmonize SHARED mpix_harmonize.c)
add_library(MPIXHarmonize::mpix-harmonize ALIAS mpix-harmonize)
target_link_libraries(mpix-harmonize PUBLIC MPITS::mpits MPI::MPI_C)
set_target_properties(mpix-harmonize PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(mpix-harmonize PROPERTIES PUBLIC_HEADER mpix_harmonize.h)
set_target_properties(mpix-harmonize PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_LIBDIR})

# check if we're being built as a sub-project
STRING(COMPARE EQUAL ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} MPIX_HARMONIZE_BUILD_INPLACE)
if(NOT MPIX_HARMONIZE_BUILD_INPLACE)
  include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}")
endif(NOT MPIX_HARMONIZE_BUILD_INPLACE)
include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

# make sure the include directory is part of the interface
target_include_directories(mpix-harmonize
        INTERFACE
        $<INSTALL_INTERFACE:${MPIX_HARMONIZE_INSTALL_INCLUDEDIR}>
        $<$<NOT:${MPIX_HARMONIZE_BUILD_INPLACE}>:
          $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

install(TARGETS mpix-harmonize
    EXPORT mpix-harmonize
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT mpix-harmonize # rules to export the targets built here
    FILE MPIXHarmonizeTargets.cmake
    NAMESPACE MPIXHarmonize::
    DESTINATION ${MPIX_HARMONIZE_INSTALL_CMAKEDIR})

configure_package_config_file(mpixharmonize-config.cmake.in
    ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mpixharmonize-config.cmake
    INSTALL_DESTINATION ${MPIX_HARMONIZE_INSTALL_CMAKEDIR}
    PATH_VARS   MPIX_HARMONIZE_INSTALL_LIBDIR
                MPIX_HARMONIZE_INSTALL_INCLUDEDIR
                MPIX_HARMONIZE_INSTALL_BINDIR
                MPIX_HARMONIZE_INSTALL_LIBEXECDIR
                MPIX_HARMONIZE_INSTALL_CMAKEDIR)

install(FILES ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mpixharmonize-config.cmake
        DESTINATION ${MPIX_HARMONIZE_INSTALL_CMAKEDIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mpix-harmonize.pc.in ${CMAKE_CURRENT_BINARY_DIR}/mpix-harmonize.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mpix-harmonize.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(FILES)
