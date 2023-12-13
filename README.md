# mpix-harmonize

Experimental MPI function to synchronize processes in the space and time dimension

# Installation

## Basic Build
```sh
mkdir build
cd build
cmake .
make
```

## Specific Target
If you want to install the library into a specific target folder, use
```sh
cmake -DCMAKE_INSTALL_PREFIX=<YOUR_PATH> .
```

## Time Source
The clock synchronization algorithms depend on the time source used internally.
By default, `libmpix-harmonize` will use `clock_gettime(CLOCK_MONOTONIC,..)` as its source of time. 
Then, the library uses the timing function that is provided by the MPI library, 
which may be mapped to different timing function internally.

The `libmpix-harmonize` library can also be configured to use a specific time function internally. 
Thus, in order to use `clock_gettime()` with `CLOCK_MONOTONIC` configure with
```sh
cmake -DMPITS_CLOCK=monotonic .
```
and to use `clock_gettime()` with `CLOCK_REALTIME` configure like this
```sh
cmake -DMPITS_CLOCK=realtime .
```

# Usage

A call to `MPIX_Harmonize` does a couple of things:

1) It periodically synchronizes the internal clock across all processes. By default, if the last call to `MPIX_Harmonize` has been more than 1s ago or any process has failed the previous call to `MPIX_Harmonize` the internal clock synchronization is performed.
2) All processes select an internal deadline based on the synchronized internal clocks.
3) All processes wait until that deadline before returning.

If the calling process was able to meet the deadline it will return `1` in `flag`, otherwise it returns `0`. It is important that `flag` only signals success or failure of the calling process, not all processes in the communicator. Internally synchronizing this flag would again introduce skew among processes. It is left to the application to handle cases in which processes have missed the deadline are returned late.

The library will automatically adjust the slack used to determine the deadline, i.e., upon successful synchronization the slack will be reduced and if a synchronization fails the slack will be increased. Thus, spurious synchronization failures may occur if the slack is chosen too small.

## Example

The below example shows a possible use of `MPIX_Harmonize`: the benchmark performs `NUM_ITERATIONS` repetitions and in each iteration synchronizes the processes through a call to `MPIX_Harmonize`. After the call returns, a timestamp is taken, the collective operation under test (`MPI_Allreduce` in this case) is performed, and a second timestamp is taken. Using `MPI_Allreduce`, all processes determine whether this experiment was valid, i.e., whether all processes succeeded in the synchronization and no process missed the internal deadline.  If that is true, the `num_valid` counter is incremented and the measured time is added to the accumulated time. After `NUM_ITERATIONS` the number of valid experiments and the average latency of `MPI_Allreduce` are printed.

Note that this is a simplified example. Real-world benchmarks may want to adjust `NUM_ITERATIONS` dynamically and/or combine multiple experiements in a single check, as it's done in the OSU benchmarks included in this repository.

```C
int flag, num_valid = 0;
double t_start, t_stop, t_sum = 0.0;
for (int i = 0; i < NUM_ITERATIONS; ++i) {
  /* synchronize processe */
  MPIX_Harmonize(MPI_COMM_WORLD, &flag);
  t_start = MPI_Wtime();
  MPI_Allreduce(sendbuf, recvbuf, num_elements,
                datatype, MPI_SUM,
                MPI_COMM_WORLD));
  t_stop = MPI_Wtime();
  /* check whether this experiment is valid */
  MPI_Allreduce(MPI_IN_PLACE, &flag, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
  if (flag) { /* the experiment is valid */
    num_valid++;
    t_sum += t_stop - t_start;
  }
}
printf("MPI_Allreduce: %d valid iterations, %f average latency\n", num_valid, t_sum/num_valid);
```

# License

The 3-Clause BSD License

# Publication

Joseph Schuchart, Sascha Hunold, and George Bosilca. 2023. Synchronizing MPI Processes in Space and Time. In Proceedings of the 30th European MPI Users' Group Meeting (EuroMPI '23). Association for Computing Machinery, New York, NY, USA, Article 7, 1–11. https://doi.org/10.1145/3615318.3615325

      

