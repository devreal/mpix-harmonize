#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mpix_harmonize.h"

static bool initialized = false;
static int keyval;


static
int delete_attr_cb(MPI_Comm comm, int comm_keyval,
                   void *attribute_val, void *extra_state);

int MPIX_Harmonize(
    MPI_Comm comm,
    int *outflag)
{
    int ret;
    /* inline synchronization: create a keyval for the data we want to attach to the communicator */
    if (!initialized) {
        /* create a keyval
         * TODO: add extra state and handle it in the callbacks
         */
        ret = MPI_Comm_create_keyval(MPI_COMM_NULL_COPY_FN, &delete_attr_cb, &keyval, NULL);
        if (MPI_SUCCESS != ret) {
            return ret;
        }
        initialized = true;
    }
    int flag;
    void* data;
    ret = MPI_Comm_get_attr(comm, keyval, &data, &flag);
    if (MPI_SUCCESS != ret) {
        return ret;
    }
    if (!flag) {
        /* first call on this comm, attach a new data */
        data = malloc(sizeof(int));
        ret = MPI_Comm_set_attr(comm, keyval, data);
        if (MPI_SUCCESS != ret) {
            return ret;
        }
    }

    /* TODO: do the magic: synchronize clocks and delay return until a fixed point in time */
    ret = MPI_Barrier(comm);
    if (MPI_SUCCESS != ret) {
        return ret;
    }

    *outflag = 1;

    return MPI_SUCCESS;
}


static
int delete_attr_cb(MPI_Comm comm, int comm_keyval,
                   void *attribute_val, void *extra_state)
{
    /* TODO add cleanup once a communicator is deleted */
    free(attribute_val);
    return MPI_SUCCESS;
}


#if 0
static
int copy_attr_cb(MPI_Comm oldcomm, int comm_keyval,
                 void *extra_state, void *attribute_val_in,
                 void *attribute_val_out, int *flag)
{
    /*TODO: handle copy events */
    int **val_out = (int**)attribute_val_out;
    *val_out = malloc(sizeof(int));
    **val_out = *val_in;
    *flag = 1;
    return MPI_SUCCESS;
}
#endif // 0