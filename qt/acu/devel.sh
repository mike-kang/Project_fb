_PATH=$(pwd)
LIB_PATH=${_PATH}/../..

LD_LIBRARY_PATH=${LIB_PATH}:${LIB_PATH}/tools/:${LIB_PATH}/camera/:${LIB_PATH}/inih_r29/ ${_PATH}/acu -c FID.test.ini
