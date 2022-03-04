#!/bin/bash

echo "Running basic make check in examples dir..."
cd examples/
make check
# diff argon_108.dat ../reference/argon_108.dat
cd ..

export OMP_DYNAMIC=FALSE

echo "Running make check in examples dir for MPI..."
cd examples/
make -f makefile_mpi.mk
# diff argon_108.dat ../reference/argon_108.dat
cd ..
