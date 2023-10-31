#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "mpits.h"
#include "mpix_harmonize.h"

static int keyval;

static mpits_clocksync_t cs;

static double bcast_time;


static
int delete_attr_cb(MPI_Comm comm, int comm_keyval,
                   void *attribute_val, void *extra_state);

static double get_bcast_time(MPI_Comm comm);

static int initialize_harmonize();

static void init_mpits();

enum {
    MPIX_HARMONIZE_SYNC_EXPIRED     = 1<<0,
    MPIX_HARMONIZE_LAST_SYNC_FAILED = 1<<1,
};

typedef struct mpix_harmonize_state_t {
    int sync_failed;
    int comm_rank;
    double last_sync_ts;
    double barrier_ts_slack;
} mpix_harmonize_state_t;

static int get_harmonize_state(MPI_Comm comm, mpix_harmonize_state_t** data);

static void sync_clocks(mpix_harmonize_state_t* state, MPI_Comm comm);

int MPIX_Harmonize(
    MPI_Comm comm,
    int *outflag)
{
    int ret;
    int flag;

    ret = initialize_harmonize();
    if (MPI_SUCCESS != ret) {
        return ret;
    }

    mpix_harmonize_state_t* data;
    ret = get_harmonize_state(comm, &data);
    if (MPI_SUCCESS != ret) {
        return ret;
    }

    /* a process forces a resync if:
     * 1) it thinks the last sync was longer than 1s ago; or
     * 2) the last sync failed.
     **/
    int need_resync = data->sync_failed ? MPIX_HARMONIZE_LAST_SYNC_FAILED : 0;
    if (MPITS_Clocksync_get_time(&cs) > (data->last_sync_ts + 1.0)) {
        need_resync |= MPIX_HARMONIZE_SYNC_EXPIRED;
    }
    double barrier_stamp = 0.0;
    if (data->comm_rank == 0) {
        ret = MPI_Reduce(MPI_IN_PLACE, &need_resync, 1, MPI_INT, MPI_MAX, 0, comm);
        if (MPI_SUCCESS != ret) {
            fprintf(stderr, "MPI_Reduce returned %d\n", ret);
            return ret;
        }
        if (need_resync) {
            barrier_stamp = -1.0;
            if (need_resync & MPIX_HARMONIZE_LAST_SYNC_FAILED) {
                /* increase barrier time by 1.5x*/
                data->barrier_ts_slack *= 1.5;
            }
        } else {
            barrier_stamp = MPITS_Clocksync_get_time(&cs) + data->barrier_ts_slack;
        }
    } else {
        ret = MPI_Reduce(&need_resync, NULL, 1, MPI_INT, MPI_MAX, 0, comm);
        if (MPI_SUCCESS != ret) {
            fprintf(stderr, "MPI_Reduce returned %d\n", ret);
            return ret;
        }
    }
    ret = MPI_Bcast(&barrier_stamp, 1, MPI_DOUBLE, 0, comm);
    if (MPI_SUCCESS != ret) {
        fprintf(stderr, "MPI_Bcast returned %d\n", ret);
        return ret;
    }
    if (barrier_stamp < 0.0) {
        /* resync required */
        sync_clocks(data, comm);
        /* determine a new barrier timestamp */
        barrier_stamp = data->last_sync_ts + data->barrier_ts_slack;
        ret = MPI_Bcast(&barrier_stamp, 1, MPI_DOUBLE, 0, comm);
        if (MPI_SUCCESS != ret) {
            fprintf(stderr, "MPI_Bcast returned %d\n", ret);
            return ret;
        }
    }

    /* check if we are within the time epoch */
    if(MPITS_Clocksync_get_time(&cs) > barrier_stamp ) {
        *outflag = 0;
        data->sync_failed = 1;
    } else {
        *outflag = 1;
        data->sync_failed = 0;
    }

    /* wait for the epoch to end */
    while(MPITS_Clocksync_get_time(&cs) <= barrier_stamp);

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

//static void reprompi_check_and_override_lib_env_params(int *argc, char ***argv) {
//    char *env = getenv("REPROMPI_LIB_PARAMS");
//    char **argvnew;
//
//    if( env != NULL ) {
//        char *token;
//        //printf("env:%s\n", env);
//        *argc = compute_argc(env) + 1;  // + 1 is for argv[0], which we'll copy
//        //printf("argc: %d\n", *argc);
//
////    printf("(*argv)[0]=%s\n", (*argv)[0]);
//
//        //  TODO: we should probably free the old argv
//        argvnew = (char**)malloc(*argc * sizeof(char**));
//        // copy old argv[0]
//
//        char *fake_arg0 = (char*) malloc(50*sizeof(char));
//        strcpy(fake_arg0, "mpi_time_barrier");
//
//        argvnew[0] = fake_arg0;
//
//    //printf("argvnew[0]=%s\n", argvnew[0]);
//
//        token = strtok(env, " ");
//        if( token != NULL ) {
//      //printf("token: %s\n", token);
//            argvnew[1] = token;
//      //printf("argvnew[1]=%s\n", argvnew[1]);
//            for(int i=2; i<*argc; i++) {
//                token = strtok(NULL, " ");
//                if( token != NULL ) {
//          //printf("token: %s\n", token);
//                    argvnew[i] = token;
//                }
//            }
//        }
//
//        *argv = argvnew;
//    }
//
//}

static void init_mpits() {

    MPITS_Init(MPI_COMM_WORLD, &cs);
    MPITS_Clocksync_init(&cs);

    bcast_time = get_bcast_time(MPI_COMM_WORLD);
}

static int initialize_harmonize() {
    int ret;
    static bool initialized = false;
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
            init_mpits();
            initialized = true;
        }
        pthread_mutex_unlock(&init_mtx);
    }
    return MPI_SUCCESS;
}

static int get_harmonize_state(MPI_Comm comm, mpix_harmonize_state_t** data_ptr)
{
    int ret, flag;
    mpix_harmonize_state_t *data;
    ret = MPI_Comm_get_attr(comm, keyval, &data, &flag);
    if (MPI_SUCCESS != ret) {
        return ret;
    }
    if (!flag) {
        /* first call on this comm, attach a new data */
        data = calloc(1, sizeof(mpix_harmonize_state_t));
        MPI_Comm_rank(comm, &data->comm_rank);
        data->barrier_ts_slack = 2*bcast_time; // start with 2x the bcast time
        ret = MPI_Comm_set_attr(comm, keyval, data);
        if (MPI_SUCCESS != ret) {
            return ret;
        }
        /* initial clock sync */
        sync_clocks(data, comm);
    }
    *data_ptr = data;
    return ret;
}

static void sync_clocks(mpix_harmonize_state_t* state, MPI_Comm comm)
{
    /* resync required */
    MPITS_Clocksync_sync(&cs);
    /* streamline sync jitter */
    int zero = 0, dummy;
    MPI_Reduce(&dummy, &zero, 1, MPI_INT, MPI_SUM, 0, comm);
    state->last_sync_ts = MPITS_Clocksync_get_time(&cs);
}

static double get_bcast_time(MPI_Comm comm) {
  int n_bcasts = 20;
  double bcast_data = 1.0;  // same as timestamp later
  double max_avg_time = 0.0;
  double timestamp;

  for(int i=0; i<n_bcasts; i++) {
    timestamp = MPITS_get_time();
    MPI_Bcast(&bcast_data, 1, MPI_DOUBLE, 0, comm);
    max_avg_time += MPITS_get_time() - timestamp;
  }
  max_avg_time /= n_bcasts;
  MPI_Allreduce(MPI_IN_PLACE, &max_avg_time, 1, MPI_DOUBLE, MPI_MAX, comm);
  return max_avg_time;
}
