#ifndef MPIX_HARMONIZE_H
#define MPIX_HARMONIZE_H

#include <mpi.h>


/**
 * A synchronizing collective operation over the group of comm.
 * Processes will synchronize clocks internally and make an attempt
 * to delay their return until a point in time where all processes
 * return from the function at the same real time.
 * Thus, in addition to the synchronization in space that is performed
 * by an MPI_Barrier, this function also synchronizes processes in time,
 * making sure that all processes continue their exection at the same time.
 *
 * If a process detects that it was unable to meet the time-synchronization
 * requirement, it will set flag to 0. It is left to the application to handle
 * such case.
 * Otherwise flag is set to 1.
 */
int MPIX_Harmonize(
    MPI_Comm comm,
    int *outflag);

#endif // MPIX_HARMONIZE_Hß