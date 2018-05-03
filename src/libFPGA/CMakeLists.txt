find_package(OpenCV REQUIRED)

set(LIB_NAME FPGA)
set(ALTERA_PATH /home/sh/intelFPGA_pro/16.1)
include_directories(${ALTERA_PATH}/hld/host/include)
add_definitions(-DFPGA_DEVICE -std=c++11 -Wall -g -fPIC -lm)

include_directories(${OpenCV_INCLUDE_DIRS})

link_directories(
    ${ALTERA_PATH}/hld/board/de5a_net_i2/linux64/lib 
    ${ALTERA_PATH}/hld/host/linux64/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(src SRC_LIST)

add_library(${LIB_NAME} SHARED ${SRC_LIST})

target_link_libraries(${LIB_NAME} alteracl)
target_link_libraries(${LIB_NAME} terasic_a10_mmd)
target_link_libraries(${LIB_NAME} elf)
target_link_libraries(${LIB_NAME} ${OpenCV_LIBS})

# add_executable(test main.cpp)
# target_link_libraries(test ${LIB_NAME} )

