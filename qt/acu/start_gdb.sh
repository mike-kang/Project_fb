LIB_PATH=/home/pi/Project_il

LD_LIBRARY_PATH=${LIB_PATH}:${LIB_PATH}/tools/:${LIB_PATH}/camera/:${LIB_PATH}/inih_r29/arch/armv6l/ gdb -c core ./acu
