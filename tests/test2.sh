#!/bin/sh
cur_dir=$(pwd)
echo -e "\n\n **************** Test 2 ****************"
echo -e "\nCreating Producer: RED; n_items = 50"
echo -e "\nCreating Consumer: C1; n_items = 10\n"
$cur_dir/../producer/producer 50 RED &
PID1=$!
$cur_dir/../consumer/consumer 10 C1 &
PID2=$!
wait "$PID1" "$PID2"
