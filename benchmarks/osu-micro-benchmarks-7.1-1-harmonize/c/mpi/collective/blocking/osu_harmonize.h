
#ifndef MPIX_HARMONIZE_OSU_HARMONIZE_H
#define MPIX_HARMONIZE_OSU_HARMONIZE_H

#include <mpix_harmonize.h>

#define HARMONIZE_DEFINE \
const int CHECK_EVERY=16; \
int flag; \
int iterations = 0;

#define HARMONIZE_INIT \
double check_wtime[CHECK_EVERY]; \
int check_flags[CHECK_EVERY];

#define HARMONIZE_PRE_MEASURE_LOOP \
iterations = 0;

#define HARMONIZE_BARRIER \
MPI_CHECK(MPIX_Harmonize(MPI_COMM_WORLD, &flag));

#define HARMONIZE_LOOP_CHECK \
                    int check_idx = (i-options.skip) % CHECK_EVERY; \
                    check_flags[check_idx] = flag; \
                    check_wtime[check_idx] = t_stop - t_start; \
                    if (check_idx == (CHECK_EVERY-1) || i == (options.iterations + options.skip - 1)) { \
                        MPI_CHECK(MPI_Allreduce(MPI_IN_PLACE, check_flags, check_idx+1, MPI_INT, \
                                                MPI_MIN, MPI_COMM_WORLD)); \
                        for (int k = 0; k <= check_idx; ++k) { \
                            if (check_flags[k]) { \
                                timer += check_wtime[k]; \
                                iterations++; \
                            } else { \
                            } \
                        } \
                    }

#endif //MPIX_HARMONIZE_OSU_HARMONIZE_H
