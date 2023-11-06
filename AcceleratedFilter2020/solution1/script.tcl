############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project AcceleratedFilter2020
add_files AcceleratedFilter2020/AccelFilterProc.hpp
add_files AcceleratedFilter2020/AcceleratedFilter.cpp
add_files AcceleratedFilter2020/AcceleratedFilter.h
add_files -tb AcceleratedFilter2020/testbench/AcceleratedFilter_tb.cpp
add_files -tb AcceleratedFilter2020/testbench/Mahmod_100x300.png
add_files -tb AcceleratedFilter2020/testbench/Mahmod_241x322.png
add_files -tb AcceleratedFilter2020/testbench/Mahmod_TestImg.png
add_files -tb AcceleratedFilter2020/testbench/lenna.png
add_files -tb AcceleratedFilter2020/testbench/lenna_128x128.png
open_solution "solution1" -flow_target vivado
set_part {xczu3eg-sbva484-1-e}
create_clock -period 10 -name default
#source "./AcceleratedFilter2020/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
