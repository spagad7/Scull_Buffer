#!/bin/sh
cur_dir=$(pwd)
echo -e "\n\n **************** Test 4 ****************"
echo -e "\nCreating Producer: BLUE; n_items = 50"
echo -e "\nCreating Producer: GREEN; n_items = 50"
echo -e "\nCreating Consumer: C1; n_items = 200\n"
$cur_dir/../../producer/producer 50 BLUE &
PID1=$!
$cur_dir/../../producer/producer 50 GREEN &
PID2=$!
$cur_dir/../../consumer/consumer 200 C1 &
PID3=$!
wait "$PID1" "$PID2" "$PID3"
