#!/bin/sh

module="scull_buffer"

if [ "$(id -u)" != "0" ]; then
	echo "Please execute the script with super user permissions."
	exit 1
fi

echo -e "\nLoading scull_buffer module\n"
chmod 755 ./scull_buffer/*

# if scull_buffer is already loaded unload it using scull_unload script
if lsmod | grep "$module" &> /dev/null ; then
    sh ./scull_buffer/scull_unload
fi

# load scull_buffer module
sh ./scull_buffer/scull_load
chmod 777 /dev/scull*

echo -e "\nExecuting test cases\n"
sleep 1

echo -e "\n Starting test 1\n"
( . ./tests/test1.sh )
echo -e "\n Test 1 finished\n"
sleep 1

echo -e "\n Starting test 2\n"
( . ./tests/test2.sh )
echo -e "\n Test 2 finished\n"
sleep 1

echo -e "\n Starting test 3\n"
( . ./tests/test3.sh )
echo -e "\n Test 3 finished\n"
sleep 1

echo -e "\n Starting test 4\n"
( . ./tests/test4.sh )
echo -e "\n Test 4 finished\n"
sleep 1

echo -e "\n Starting test 5\n"
( . ./tests/test5.sh )
echo -e "\n Test 5 finished\n"
sleep 1

echo -e "\nAll tests done\n"
echo -e "Unloading the scull_buffer module."
sh ./scullbuffer/scull_unload
