#!/bin/sh
dir=$(pwd)
echo -e "\n\n******************************** test case 3 ********************************\n"

echo -e "\tcreating producer and consumer processes..\n"
.$dir/../producer/producer 50 BLACK &
PID1=$!
.$dir/../consumer/consumer 100 A &
PID2=$!
wait "$PID1" "$PID2"
