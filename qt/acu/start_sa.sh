_PATH=/home/pi/acufb
LIB_PATH=${_PATH}/libs/

LD_LIBRARY_PATH=${LIB_PATH}:${LIB_PATH}/tools/:${LIB_PATH}/inih_r29/ xinit ${_PATH}/acu $* -- :1
