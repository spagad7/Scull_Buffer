#!/bin/sh
cur_dir=$(pwd)
echo -e "\n\n******************************** test case 5 ********************************\n"

echo -e "\tcreating producer and consumer processes..\n"
$cur_dir/../producer/producer 50 BLACK &
PID1=$!
$cur_dir/../consumer/consumer 25 A &
PID2=$!
$cur_dir/../consumer/consumer 25 B &
PID3=$!
wait "$PID1" "$PID2" "$PID3"
