#!/bin/sh
cur_dir=$(pwd)
echo -e "\n\n******************************** test case 1 ********************************\n"

echo -e "\tcreating producer and consumer processes..\n"
$cur_dir/../producer/producer 50 BLACK &
PID1=$!
sleep 2
$cur_dir/../consumer/consumer 50 A &
PID2=$!
wait "$PID1" "$PID2"
