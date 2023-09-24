cmake_minimum_required(VERSION 3.22)
project(mpix-harmonize VERSION 0.1 LANGUAGES C DESCRIPTION "Implementation of MPIX_Harmonize")

include(GNUInstallDirs)
include(ExternalProject)

find_package(MPI REQUIRED)

set(MPITS_CMAKE_ARGS "")
set(MPITS_CLOCK "mpi_wtime" CACHE STRING "time source in libmpits")
set_property(CACHE MPITS_CLOCK PROPERTY STRINGS mpi_wtime monotonic realtime)
message(STATUS "mpits clock: ${MPITS_CLOCK}")

if(${MPITS_CLOCK} STREQUAL "mpi_wtime")
    # default, do nothing
elseif(${MPITS_CLOCK} STREQUAL "monotonic")
    set(MPITS_CMAKE_ARGS "-DENABLE_GETTIME_MONOTONIC=ON ${MPITS_CMAKE_ARGS}")
elseif(${MPITS_CLOCK} STREQUAL "realtime")
    set(MPITS_CMAKE_ARGS "-DENABLE_GETTIME_REALTIME=ON ${MPITS_CMAKE_ARGS}")
else()
    message(FATAL_ERROR "Invalid value for MPITS_CLOCK: ${MPITS_CLOCK}. Allowed values are:  mpi_wtime, monotonic, or realtime.")
endif()

ExternalProject_Add(
        mpits_project
        PREFIX          ${CMAKE_BINARY_DIR}/_deps
        GIT_REPOSITORY  git@github.com:hunsa/mpi-time-sync.git
        GIT_TAG         main
        CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/_deps ${MPITS_CMAKE_ARGS}
)

# Get properties of the external project
ExternalProject_Get_Property(mpits_project SOURCE_DIR BINARY_DIR INSTALL_DIR)
#message(STATUS "Project properties: ${SOURCE_DIR} ${BINARY_DIR} ${INSTALL_DIR}")

link_directories(${INSTALL_DIR}/lib)
include_directories(${INSTALL_DIR}/include)


add_library(mpits_lib SHARED IMPORTED)
add_dependencies(mpits_lib mpits_project)


#set_target_properties(mpits_lib PROPERTIES
#        IMPORTED_LOCATION ${INSTALL_DIR}/lib/libmpits${CMAKE_SHARED_LIBRARY_SUFFIX}
#        INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
#)

add_library(mpix-harmonize SHARED mpix_harmonize.c)
target_link_libraries(mpix-harmonize PUBLIC mpits  MPI::MPI_C)
set_target_properties(mpix-harmonize PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(mpix-harmonize PROPERTIES PUBLIC_HEADER mpix_harmonize.h)
set_target_properties(mpix-harmonize PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(mpix-harmonize.pc.in mpix-harmonize.pc @ONLY)

install(FILES
        ${CMAKE_BINARY_DIR}/_deps/lib/libmpits${CMAKE_SHARED_LIBRARY_SUFFIX}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS mpix-harmonize
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/mpix-harmonize.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(FILES)
