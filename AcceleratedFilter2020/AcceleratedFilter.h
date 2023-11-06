/*
*/

#ifndef SRC_AcceleratedFilter_H_
#define SRC_AcceleratedFilter_H_

#include <stdint.h>

#include <hls_stream.h>
#include <ap_axi_sdata.h>

#include "AccelFilterProc.hpp"

#define MAX_WIDTH  1280 // 300 //   512 // 1288
#define MAX_HEIGHT 960  // 100 //   512 // 964

//--- for test bench
#define INPUT_IMAGE   "Mahmod_TestImg.png"  // "Mahmod_100x300.png" // "Mahmod_241x322.png" //"lenna.png" // "lenna_128x128.png" //
#define OUTPUT_IMAGE "out.png"


void AcceleratedFilter(hls::stream<hlsAccelFiltProc::ImAxis<24> >& axis_in, hls::stream<hlsAccelFiltProc::ImAxis<24> >& axis_out);


#endif /* SRC_AcceleratedFilter_H_ */
