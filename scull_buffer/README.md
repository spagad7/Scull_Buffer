# scull_buffer
Character device for synchronizing producer and consumer

Compiling and installing the scull_buffer device:
--------------------------------------------------
1. Execute Make:
   make
2. If scull devices already exist then unload them first.
   sudo ./scull_unload
   This may ask you for your root password.
3. Load scull devices
   sudo ./scull_load
   This will create s scull_buffer device
4. Check that devices are properly created.
   ls -ltr /dev/scull*
5. Change device permissions to 777
   sudo chmod 777 /dev/scull*
