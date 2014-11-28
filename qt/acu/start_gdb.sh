LIB_PATH=/home/pi/acufb
LD_LIBRARY_PATH=${LIB_PATH}:${LIB_PATH}/tools/:${LIB_PATH}/inih_r29/arch/armv6l/ gdb -c core ./acu
