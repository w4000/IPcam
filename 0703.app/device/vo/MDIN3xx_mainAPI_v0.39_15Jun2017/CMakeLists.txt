cmake_minimum_required(VERSION 2.6)

set(AUTHOR_NAME "ppdha82")
project(MDIN325A)
set(HISI_COMPILER /opt/hisi-linux/arm-hisiv500-linux)
set(CMAKE_C_COMPILER ${HISI_COMPILER}/bin/arm-hisiv500-linux-uclibcgnueabi-gcc)

include_directories(./)
set(SRCS_FILES app325a.c mdin3xx.c mdinaux.c mdinbus.c mdincoef.c mdindly.c mdinfrmt.c mdinhtx.c mdini2c.c mdinipc.c mdinpci.c)

add_executable(app325a ${SRCS_FILES})
add_definitions(-DFOR_TEST)
