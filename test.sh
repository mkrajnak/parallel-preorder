#!/bin/bash

# Check args
if [ $# -lt 1 ];then
    echo "Usage: ./test.sh <String_to_preorder>";
    exit 1;
fi;
# cpus=$(echo -n $1 | wc -c);
cpus=4

mpic++ --prefix /usr/local/share/OpenMPI -o pro pro.cpp
# Check if translation was successfull to avoid to many err messages
compile=$?;
if [[ $compile != 0 ]];then
  exit $compile;
fi;

# Run
mpirun --prefix /usr/local/share/OpenMPI -np $cpus pro $1;

# Cleanup
rm -f pro numbers