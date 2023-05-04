cmake_minimum_required(VERSION 3.22)
project(mpix-harmonize VERSION 0.1 LANGUAGES C DESCRIPTION "Implementation of MPIX_Harmonize")

include(GNUInstallDirs)

find_package(MPI REQUIRED)


# pull in reprompi
include(FetchContent)
set(INCLUDE_PLATFORM_CONFIG_FILE "${CMAKE_BINARY_DIR}/_deps/reprompi-src/platform_files/default.cmake")
set(COMPILE_BENCH_LIBRARY ON CACHE BOOL "Compile Bench Library")
set(ENABEL_GETTIME_MONOTONIC ON CACHE BOOL "Use gettime MONOTONIC")
set(COMPILE_SANITY_CHECK_TESTS ON CACHE BOOL "Compile sanity checks")
FetchContent_Declare(
    reproMPI
    GIT_REPOSITORY      git@github.com:hunsa/reprompi-dev-private.git
    GIT_TAG             time_barrier
)
FetchContent_MakeAvailable(reproMPI)

add_library(mpix-harmonize SHARED mpix_harmonize.c)
target_link_libraries(mpix-harmonize PUBLIC reproMPIbench MPI::MPI_C)
set_target_properties(mpix-harmonize PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(mpix-harmonize PROPERTIES PUBLIC_HEADER mpix_harmonize.h)

configure_file(mpix-harmonize.pc.in mpix-harmonize.pc @ONLY)

install(TARGETS mpix-harmonize
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/mpix-harmonize.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(FILES )
