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


# License

The 3-Clause BSD License

# Publication

Joseph Schuchart, Sascha Hunold, and George Bosilca. 2023. Synchronizing MPI Processes in Space and Time. In Proceedings of the 30th European MPI Users' Group Meeting (EuroMPI '23). Association for Computing Machinery, New York, NY, USA, Article 7, 1–11. https://doi.org/10.1145/3615318.3615325

      

