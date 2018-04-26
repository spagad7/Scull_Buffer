#!/bin/sh
cur_dir=$(pwd)
echo -e "\n\n **************** Test 1 ****************"
echo -e "\nCreating Producer: WHITE; n_items = 50"
echo -e "\nCreating Consumer: C1; n_items = 50\n"
$cur_dir/../../producer/producer 50 WHITE &
PID1=$!
$cur_dir/../../consumer/consumer 50 C1 &
PID2=$!
wait "$PID1" "$PID2"
