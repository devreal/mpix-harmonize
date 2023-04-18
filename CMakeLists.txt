cmake_minimum_required(VERSION 3.22)
project(mpix-harmonize VERSION 0.1 LANGUAGES C DESCRIPTION "Implementation of MPIX_Harmonize")

include(GNUInstallDirs)

find_package(MPI REQUIRED)


# pull in reprompi
if ( 0 )
include(ExternalProject)
ExternalProject_Add(reprompi
        GIT_REPOSITORY git@github.com:hunsa/reprompi-dev-private.git
        GIT_TAG time_barrier
        #CMAKE_ARGS -DCOMPILE_BENCH_LIBRARY=ON -DOPTION_ENABLE_DEBUGGING=ON -DENABLE_GETTIME_REALTIME=ON
        CMAKE_ARGS -DCOMPILE_BENCH_LIBRARY=ON -DENABLE_GETTIME_MONOTONIC=ON -DCOMPILE_SANITY_CHECK_TESTS=ON
        PREFIX ${CMAKE_BINARY_DIR}/_deps/reprompi
        INSTALL_DIR ${CMAKE_INSTALL_DIR}
        )

ExternalProject_Get_Property(reprompi install_dir)
set(REPROMPI_LIB_DIR ${install_dir}/src/reprompi-build/lib)
set(REPROMPI_INC_DIR ${install_dir}/src/reprompi-build/include)
LINK_DIRECTORIES(${REPROMPI_LIB_DIR})
#INCLUDE_DIRECTORIES("${install_dir}/src/reprompi")
INCLUDE_DIRECTORIES("${install_dir}/src/reprompi-build/include")
else (0)

include(FetchContent)
set(INCLUDE_PLATFORM_CONFIG_FILE "${CMAKE_BINARY_DIR}/_deps/reprompi-src/platform_files/default.cmake")
set(COMPILE_BENCH_LIBRARY ON)
set(ENABEL_GETTIME_MONOTONIC ON)
set(COMPILE_SANITY_CHECK_TESTS ON)
FetchContent_Declare(
    reproMPI
    GIT_REPOSITORY      git@github.com:hunsa/reprompi-dev-private.git
    GIT_TAG             time_barrier
)
FetchContent_MakeAvailable(reproMPI)
FetchContent_GetProperties(reproMPI
    SOURCE_DIR REPROMPI_SOURCE_DIR
    BINARY_DIR REPROMPI_BINARY_DIR
    )

endif (0)


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