/*
*/

#include <hls_opencv.h>
#include "../src/AcceleratedFilter.h"

int main() {
    hls::stream<ap_axiu<24,1,1,1> > gen_axis_in, gen_axis_out;
    hls::stream<hlsAccelFiltProc::ImAxis<24> > im_axis_in, im_axis_out;
    uint8_t itration;//KhS
	for(int itration = 0; itration < 3; itration++) {//KhS
		// read image
		cv::Mat src = cv::imread(INPUT_IMAGE);
		cv::Mat dst = src;

		// cv::Mat -> AXI4-Stream
		cvMat2AXIvideo(src, gen_axis_in);

		// convert axis type (ap_axiu -> hlsAccelFiltProc::ImAxis)
		ap_axiu<24,1,1,1> gen_axis_reader;
		hlsAccelFiltProc::ImAxis<24> im_axis_writer;

		for(int yi = 0; yi < MAX_HEIGHT; yi++) {
			for(int xi = 0; xi < MAX_WIDTH; xi++) {
				gen_axis_in >> gen_axis_reader;

				im_axis_writer.data = gen_axis_reader.data;
				im_axis_writer.user = gen_axis_reader.user;
				im_axis_writer.last = gen_axis_reader.last;

				im_axis_in << im_axis_writer;
			}
		}

		// canny edge detection
		AcceleratedFilter(im_axis_in, im_axis_out);

		// convert axis type (hlsAccelFiltProc::ImAxis -> ap_axiu)
		ap_axiu<24,1,1,1> gen_axis_writer;
		hlsAccelFiltProc::ImAxis<24> im_axis_reader;

		for(int yi = 0; yi < MAX_HEIGHT; yi++) {
			for(int xi = 0; xi < MAX_WIDTH; xi++) {
				im_axis_out >> im_axis_reader;

				gen_axis_writer.data = im_axis_reader.data;
				gen_axis_writer.user = im_axis_reader.user;
				gen_axis_writer.last = im_axis_reader.last;

				gen_axis_out << gen_axis_writer;
			}
		}

		// AXI4-Stream -> cv::Mat
		AXIvideo2cvMat(gen_axis_out, dst);

		// write image
		cv::imwrite(OUTPUT_IMAGE, dst);
	}
    return 0;
}
