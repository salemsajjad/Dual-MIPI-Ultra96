/*
*/

#include "AcceleratedFilter.h"

using namespace hls;
using namespace hlsAccelFiltProc;

uint8_t fifo1[MAX_WIDTH * MAX_HEIGHT];
uint8_t fifo2[MAX_WIDTH * MAX_HEIGHT];


void AcceleratedFilter(stream<ImAxis<24> >& axis_in, stream<ImAxis<24> >& axis_out){


    #pragma HLS INTERFACE axis port=axis_in
    #pragma HLS INTERFACE axis port=axis_out

    #pragma HLS INTERFACE ap_ctrl_none port=return

    #pragma HLS DATAFLOW

    #pragma HLS STREAM variable=fifo1 depth=1 dim=1
    #pragma HLS STREAM variable=fifo2 depth=1 dim=1

    // AXI4-Stream -> GrayScale image
    AccelFilterProc::AXIS2GrayArray<MAX_WIDTH, MAX_HEIGHT>(axis_in, fifo1);

    // CustomFilter
    AccelFilterProc::CustomFilter<MAX_WIDTH, MAX_HEIGHT>(fifo1, fifo2);

    // GrayScale image -> AXI4-Stream
    AccelFilterProc::GrayArray2AXIS<MAX_WIDTH, MAX_HEIGHT>(fifo2, axis_out);
}
