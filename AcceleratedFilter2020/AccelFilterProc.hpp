/*
*/

#ifndef SRC_ACCEL_FILTER_PROC_HPP_
#define SRC_ACCEL_FILTER_PROC_HPP_

#include <stdint.h>

#include <hls_stream.h>
#include <hls_math.h>

namespace hlsAccelFiltProc {
    // struct for image flowing through AXI4-Stream
    template<int D>
    struct ImAxis {
        ap_uint<D> data;
        ap_uint<1> user;
        ap_uint<1> last;
    };

    class AccelFilterProc {
        public:
        // AXI4-Stream -> GrayScale image
        template<uint32_t WIDTH, uint32_t HEIGHT>
        static void AXIS2GrayArray(hls::stream<ImAxis<24> >& axis_src, uint8_t* dst);
        // CustomFilter
        template<uint32_t WIDTH, uint32_t HEIGHT>
        static void CustomFilter(uint8_t* src, uint8_t* dst);
        // GrayScale image -> AXI4-Stream
        template<uint32_t WIDTH, uint32_t HEIGHT>
        static void GrayArray2AXIS(uint8_t* src, hls::stream<ImAxis<24> >& axis_dst);
    };

    template<uint32_t WIDTH, uint32_t HEIGHT>
    inline void AccelFilterProc::AXIS2GrayArray(hls::stream<ImAxis<24> >& axis_src, uint8_t* dst) {
        ImAxis<24> axis_reader; // for read AXI4-Stream
        bool sof = false;        // Start of Frame
        bool eol = false;        // End of Line

        // wait for the user signal to be asserted
        while (!sof) {
        	#pragma HLS PIPELINE II=1
            #pragma HLS LOOP_TRIPCOUNT avg=0 max=0

            axis_src >> axis_reader;
            sof = axis_reader.user.to_int();
        }

        // image proc loop
        for(int yi = 0; yi < HEIGHT; yi++) {
            eol = false;
            for(int xi = 0; xi < WIDTH; xi++) {
            	#pragma HLS PIPELINE II=1
                #pragma HLS LOOP_FLATTEN off

                // get pix until the last signal to be asserted
                if(sof || eol) {
                    // when frame is started (first pix have already latched)
                    // or
                    // when WIDTH param set more than actual frame size
                    sof = false;
                    eol = axis_reader.last.to_int();
                }
                else {
                    axis_src >> axis_reader;
                    eol = axis_reader.last.to_int();
                }

                //--- grayscale processing
                int pix_gray;

                // Y = B*0.144 + G*0.587 + R*0.299
                pix_gray = 9437*(axis_reader.data & 0x0000ff)
                    + 38469*((axis_reader.data & 0x00ff00) >> 8 )
                    + 19595*((axis_reader.data & 0xff0000) >> 16);

                pix_gray >>= 16;

                // to consider saturation
                if(pix_gray < 0) {
                    pix_gray = 0;
                }
                else if(pix_gray > 255) {
                    pix_gray = 255;
                }

                // output
                dst[xi + yi*WIDTH] = pix_gray;
            }

            // when WIDTH param set less than actual frame size
            // wait for the last signal to be asserted
            while (!eol) {
            	#pragma HLS PIPELINE II=1
                #pragma HLS LOOP_TRIPCOUNT avg=0 max=0
                axis_src >> axis_reader;
                eol = axis_reader.last.to_int();
            }
        }
    }

    template<uint32_t WIDTH, uint32_t HEIGHT>
    inline void AccelFilterProc::CustomFilter(uint8_t* src, uint8_t* dst) {
        const int KERNEL_SIZE = 15;

        uint8_t line_buf[KERNEL_SIZE][WIDTH];
        uint8_t window_buf[KERNEL_SIZE][KERNEL_SIZE];

        #pragma HLS ARRAY_RESHAPE variable=line_buf complete dim=1
        #pragma HLS ARRAY_PARTITION variable=window_buf complete dim=0

        //-- 15x15  kernel (8bit left shift)
        const int FilterKERNEL[KERNEL_SIZE][KERNEL_SIZE] = { {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1, -224,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
															 {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1} };


        #pragma HLS ARRAY_PARTITION variable=FilterKERNEL complete dim=0

        // image proc loop
        for(int yi = 0; yi < HEIGHT; yi++) {
            for(int xi = 0; xi < WIDTH; xi++) {
            	#pragma HLS PIPELINE II=1
                #pragma HLS LOOP_FLATTEN off

                //--- CustomFilter
                int pix_filter;

                //-- line buffer
                for(int yl = 0; yl < KERNEL_SIZE - 1; yl++) {
                    line_buf[yl][xi] = line_buf[yl + 1][xi];
                }

                // write to line buffer
                line_buf[KERNEL_SIZE - 1][xi] = src[xi + yi*WIDTH];

                //-- window buffer
                for(int yw = 0; yw < KERNEL_SIZE; yw++) {
                    for(int xw = 0; xw < KERNEL_SIZE - 1; xw++) {
                        window_buf[yw][xw] = window_buf[yw][xw + 1];
                    }
                }

                // write to window buffer
                for(int yw = 0; yw < KERNEL_SIZE; yw++) {
                    window_buf[yw][KERNEL_SIZE - 1] = line_buf[yw][xi];
                }

                //-- convolution
                pix_filter = 0;
                for(int yw = 0; yw < KERNEL_SIZE; yw++) {
                    for(int xw = 0; xw < KERNEL_SIZE; xw++) {
                        pix_filter += window_buf[yw][xw] * FilterKERNEL[yw][xw];
                    }
                }

                // 8bit right shift
                //pix_filter >>= 8;

                // output
                //dst[xi + yi*WIDTH] = pix_filter;
                if (pix_filter >=500){
                	dst[xi + yi*WIDTH] = 255;
                }
                else  {
                	dst[xi + yi*WIDTH] = 0;
                }
            }
        }
    }

    template<uint32_t WIDTH, uint32_t HEIGHT>
    inline void AccelFilterProc::GrayArray2AXIS(uint8_t* src, hls::stream<ImAxis<24> >& axis_dst) {
        ImAxis<24> axis_writer; // for write AXI4-Stream

        // image proc loop
        for(int yi = 0; yi < HEIGHT; yi++) {
            for(int xi = 0; xi < WIDTH; xi++) {
            	#pragma HLS PIPELINE II=1
                #pragma HLS LOOP_FLATTEN off

                unsigned int pix_out = src[xi + yi*WIDTH];
                axis_writer.data = pix_out << 16 | pix_out << 8 | pix_out;

                // assert user signal at start of frame
                if (xi == 0 && yi == 0) {
                    axis_writer.user = 1;
                }
                else {
                    axis_writer.user = 0;
                }
                // assert last signal at end of line
                if (xi == (WIDTH - 1)) {
                    axis_writer.last = 1;
                }
                else {
                    axis_writer.last = 0;
                }

                // output
                axis_dst << axis_writer;
            }
        }
    }

}

#endif /* SRC_ACCEL_FILTER_PROC_HPP_ */
