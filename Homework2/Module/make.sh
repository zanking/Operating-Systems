sudo rmmod simple_char_driver
make -C /lib/modules/$(uname -r)/build M=$PWD modules
gcc test.c
sudo insmod simple_char_driver.ko
