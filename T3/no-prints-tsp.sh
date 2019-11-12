#!/bin/bash
./tsp_mpi $1 $2 | grep -e 'Time' -e 'Best tour:' -e 'Cost'
