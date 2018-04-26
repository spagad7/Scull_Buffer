#!/bin/sh
cur_dir=$(pwd)
echo -e "\n\n **************** Test 5 ****************"
echo -e "\nCreating Producer: WHITE; n_items = 50"
echo -e "\nCreating Consumer: C1; n_items = 25"
echo -e "\nCreating Consumer: C2; n_items = 25\n"
$cur_dir/../producer/producer 50 WHITE &
PID1=$!
$cur_dir/../consumer/consumer 25 C1 &
PID2=$!
$cur_dir/../consumer/consumer 25 C2 &
PID3=$!
wait "$PID1" "$PID2" "$PID3"
