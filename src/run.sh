#!/bin/bash

export MODULE_NAME=mmap_simple;
export MODULE_FILE=mmap_simple.ko;
export TIMEOUT=500;

# run if user hits control-c
control_c()
{
  echo -en "\n Exiting .. Removing module .. \n"
  rmmod $MODULE_NAME;
  exit $?
}

# trap keyboard interrupt (control-c)
trap control_c SIGINT


if [[ $(lsmod | grep $MODULE_NAME) ]]; then
    echo "Removing module ";
    rmmod $MODULE_NAME;
fi

echo "Inserting module..";
insmod $MODULE_FILE;
echo "Running module for $TIMEOUT secs ....";
sleep $TIMEOUT;
rmmod $MODULE_NAME;


