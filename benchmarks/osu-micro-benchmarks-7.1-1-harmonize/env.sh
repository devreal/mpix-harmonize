
#export REPROMPI_LIB_PARAMS="--clock-sync=Topo2 --params=topoalg1:hca3offset@skampi_offset@5@20 --params=topoalg2:prop@0"
# export LD_LIBRARY_PATH=/sw/hawk-rh8/hlrs/spack/rev-009_2022-09-01/gsl/2.6-gcc-10.2.0-qloqbuuq/lib:/zhome/academic/HLRS/hlrs/hpcjschu/opt-hawk/mpix-harmonize/lib64:$LD_LIBRARY_PATH

#export LD_LIBRARY_PATH=/sw/hawk-rh8/hlrs/spack/rev-009_2022-09-01/gsl/2.6-gcc-10.2.0-qloqbuuq/lib:/zhome/academic/HLRS/hlrs/hpcjschu/opt-hawk/mpix-harmonize/lib64:$LD_LIBRARY_PATH
#../configure CC=scorep-mpicc CXX=scorep-mpicxx CFLAGS="-I/zhome/academic/HLRS/hlrs/hpcjschu/opt-hawk/mpix-harmonize/include/" LDFLAGS="-L/zhome/academic/HLRS/hlrs/hpcjschu/opt-hawk/mpix-harmonize/lib64 -lmpix-harmonize -lreproMPIbench -lsynclib -L/sw/hawk-rh8/hlrs/spack/rev-009_2022-09-01/gsl/2.6-gcc-10.2.0-qloqbuuq/lib -lgsl -lgslcblas"

#export MPITS_PARAMS="--clock-sync=Topo2 --params=topoalg1:hca3offset@skampi_offset@5@20 --params=topoalg2:prop@0"
export MPITS_PARAMS="--clock-sync=HCA3O --params=alg:hca3offset@skampi_offset@5@20"

# build like so
# export MPIX_PATH=$HOME/<MYPATH>/mpix-harmonize
# export MPIX_PATH="$(pwd)/../.."
# ./configure CC=mpicc CXX=mpicxx CFLAGS="-I${MPIX_PATH}" LDFLAGS="-L${MPIX_PATH} -lmpix-harmonize"
# make
# export LD_LIBRARY_PATH=$MPIX_PATH:$LD_LIBRARY_PATH
# mpirun -np 16 ./c/mpi/collective/blocking/osu_reduce -f
