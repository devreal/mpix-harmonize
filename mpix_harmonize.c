#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "reprompi_bench/sync/clock_sync/synchronization.h"
//#include "reprompi_bench/sync/process_sync/process_synchronization.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "reprompi_bench/sync/time_measurement.h"

#include "mpix_harmonize.h"

static bool initialized = false;
static int keyval;

static reprompib_sync_module_t clock_sync;


static
int delete_attr_cb(MPI_Comm comm, int comm_keyval,
                   void *attribute_val, void *extra_state);
static
void reprompi_check_and_override_lib_env_params(int *argc, char ***argv);

static void init_reprompi();

int MPIX_Harmonize(
    MPI_Comm comm,
    int *outflag)
{
    int ret;
    /* inline synchronization: create a keyval for the data we want to attach to the communicator */
    if (!initialized) {
        static pthread_mutex_t init_mtx = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&init_mtx);
        if (!initialized) {
            /* create a keyval
             * TODO: add extra state and handle it in the callbacks
             */
            ret = MPI_Comm_create_keyval(MPI_COMM_NULL_COPY_FN, &delete_attr_cb, &keyval, NULL);
            if (MPI_SUCCESS != ret) {
                return ret;
            }
            init_reprompi();
            initialized = true;
        }
        pthread_mutex_unlock(&init_mtx);
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

    clock_sync.sync_clocks();
    double barrier_stamp = clock_sync.get_global_time(REPROMPI_get_time()) + 0.001;
    ret = MPI_Bcast(&barrier_stamp, 1, MPI_DOUBLE, 0, comm);
    if (MPI_SUCCESS != ret) {
        fprintf(stderr, "MPI_Barrier returned %d\n", ret);
        return ret;
    }

    /* check if we are within the time epoch */
    if( clock_sync.get_global_time(REPROMPI_get_time()) > barrier_stamp ) {
        *outflag = 0;
    } else {
        *outflag = 1;
    }

    /* wait for the epoch to end */
    while(clock_sync.get_global_time(REPROMPI_get_time()) <=   barrier_stamp);

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


static int compute_argc(char *str) {
    size_t i;
    int cnt = 0;
    int white = 0;
    int seenword = 0;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] == ' ') {
            white = 1;
        } else {
            if( i == strlen(str) -1 &&  white == 0 ) {
                cnt++;
            } else if (white == 1) {
                if( seenword == 1 ) {
                    cnt++;
                }
            }
            white = 0;
            seenword = 1;
        }
    }
    return cnt;
}

static void reprompi_check_and_override_lib_env_params(int *argc, char ***argv) {
    char *env = getenv("REPROMPI_LIB_PARAMS");
    char **argvnew;

    if( env != NULL ) {
        char *token;
        //printf("env:%s\n", env);
        *argc = compute_argc(env) + 1;  // + 1 is for argv[0], which we'll copy
        //printf("argc: %d\n", *argc);

//    printf("(*argv)[0]=%s\n", (*argv)[0]);

        //  TODO: we should probably free the old argv
        argvnew = (char**)malloc(*argc * sizeof(char**));
        // copy old argv[0]

        char *fake_arg0 = (char*) malloc(50*sizeof(char));
        strcpy(fake_arg0, "mpi_time_barrier");

        argvnew[0] = fake_arg0;

    //printf("argvnew[0]=%s\n", argvnew[0]);

        token = strtok(env, " ");
        if( token != NULL ) {
      //printf("token: %s\n", token);
            argvnew[1] = token;
      //printf("argvnew[1]=%s\n", argvnew[1]);
            for(int i=2; i<*argc; i++) {
                token = strtok(NULL, " ");
                if( token != NULL ) {
          //printf("token: %s\n", token);
                    argvnew[i] = token;
                }
            }
        }

        *argv = argvnew;
    }

}

static void init_reprompi() {

    int c_argc;
    char **c_argv;

    reprompi_check_and_override_lib_env_params(&c_argc, &c_argv);

    reprompib_register_sync_modules();
    reprompib_init_sync_module(c_argc, c_argv, &clock_sync);

    clock_sync.init_sync();
}