#!/bin/sh
cur_dir=$(pwd)
echo -e "\n\n **************** Test 3 ****************"
echo -e "\nCreating Producer: BLACK; n_items = 50"
echo -e "\nCreating Consumer: C1; n_items = 100\n"
$cur_dir/../../producer/producer 50 BLACK &
PID1=$!
$cur_dir/../../consumer/consumer 100 C1 &
PID2=$!
wait "$PID1" "$PID2"
