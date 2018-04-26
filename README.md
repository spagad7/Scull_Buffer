## How to compile
Run 'make' in the parent directory Scull_Buffer.
This will automatically run 'make' in the sub-directories.

## How to load and unload scull_buffer module
Navigate to scull_buffer:

`cd scull_buffer`

`sudo ./scull_load`

**Note:** if another scull_buffer module has already been loaded execute below script before loading the scull_module

`sudo ./scull_unload`


## How to run tests
Navigate to tests\test[1, 2, 3, 4, 5] folder and run the shell script. For example:

 `cd tests\test1`

and run

 `sudo ./test1.sh`

 Each producer and consumer will create their own file in their corresponding test directories.
