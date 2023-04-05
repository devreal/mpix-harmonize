cmake_minimum_required(VERSION 3.9)
project(mpix-harmonize VERSION 0.1 LANGUAGES C DESCRIPTION "Implementation of MPIX_Harmonize")

include(GNUInstallDirs)

find_package(MPI REQUIRED)

add_library(mpix-harmonize SHARED mpix_harmonize.c)
target_link_libraries(mpix-harmonize PUBLIC MPI::MPI_C)
set_target_properties(mpix-harmonize PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(mpix-harmonize PROPERTIES PUBLIC_HEADER mpix_haromize.h)

configure_file(mpix-harmonize.pc.in mpix-harmonize.pc @ONLY)

install(TARGETS mpix-harmonize
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/mpix-harmonize.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)