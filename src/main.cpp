#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>       /* sqrt */

#define USE_UNICODE_FOR_BETTER_GRAPHICS

struct winsize size;
using namespace cv;
using namespace std;

Mat rotate_frame(Mat src,double angle) {

    // get rotation matrix for rotating the image around its center in pixel coordinates
    cv::Point2f center((src.cols-1)/2.0, (src.rows-1)/2.0);
    cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
    // determine bounding rectangle, center !relevant
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
    // adjust transformation matrix
    rot.at<double>(0,2) += bbox.width/2.0 - src.cols/2.0;
    rot.at<double>(1,2) += bbox.height/2.0 - src.rows/2.0;

    cv::Mat dst;
    cv::warpAffine(src, dst, rot, bbox.size());
    return dst;
}

double color_distance (int red1, int green1, int blue1, int red2, int green2, int blue2) {
	double rmean = ( red1 + red2 )/2;
	int r = red1 - red2;
	int g = green1 - green2;
	int b = blue1 - blue2;
	double weightR = 2 + rmean/256;
	double weightG = 4.0;
	double weightB = 2 + (255-rmean)/256;
	return sqrt(weightR*r*r + weightG*g*g + weightB*b*b);
}

void draw_pixel(Mat frame,uint8_t* pixelPtr, int baseX, int baseY) {
	int channels = frame.channels();
	int pixelThreshold = 10;
	int pixelA_b = pixelPtr[(baseX)*frame.cols*channels + (baseY)*channels + 0]; // B
    int pixelA_g = pixelPtr[(baseX)*frame.cols*channels + (baseY)*channels + 1]; // G
    int pixelA_r = pixelPtr[(baseX)*frame.cols*channels + (baseY)*channels + 2]; // R
	int pixelB_b = pixelPtr[(baseX+1)*frame.cols*channels + (baseY)*channels + 0]; // B
    int pixelB_g = pixelPtr[(baseX+1)*frame.cols*channels + (baseY)*channels + 1]; // G
    int pixelB_r = pixelPtr[(baseX+1)*frame.cols*channels + (baseY)*channels + 2]; // R
	int pixelC_b = pixelPtr[(baseX)*frame.cols*channels + (baseY+1)*channels + 0]; // B
    int pixelC_g = pixelPtr[(baseX)*frame.cols*channels + (baseY+1)*channels + 1]; // G
    int pixelC_r = pixelPtr[(baseX)*frame.cols*channels + (baseY+1)*channels + 2]; // R
	int pixelD_b = pixelPtr[(baseX+1)*frame.cols*channels + (baseY+1)*channels + 0]; // B
    int pixelD_g = pixelPtr[(baseX+1)*frame.cols*channels + (baseY+1)*channels + 1]; // G
    int pixelD_r = pixelPtr[(baseX+1)*frame.cols*channels + (baseY+1)*channels + 2]; // R
	bool ABSame = color_distance(pixelA_r,pixelA_g,pixelA_b,pixelB_r,pixelB_g,pixelB_b) < pixelThreshold;
	bool CDSame = color_distance(pixelC_r,pixelC_g,pixelC_b,pixelD_r,pixelD_g,pixelD_b) < pixelThreshold;
	bool ACSame = color_distance(pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b) < pixelThreshold;
	bool BDSame = color_distance(pixelB_r,pixelB_g,pixelB_b,pixelD_r,pixelD_g,pixelD_b) < pixelThreshold;
	bool ADSame = color_distance(pixelA_r,pixelA_g,pixelA_b,pixelD_r,pixelD_g,pixelD_b) < pixelThreshold;
	bool BCSame = color_distance(pixelB_r,pixelB_g,pixelB_b,pixelC_r,pixelC_g,pixelC_b) < pixelThreshold;

	if( (ABSame && CDSame && ACSame) ||
		(ADSame && BCSame) ||
		(ACSame && BDSame) ||
		(ABSame && CDSame) ||
		(BCSame && CDSame && ACSame) ||
		(ABSame && BCSame && CDSame)) {
		printf("\033[48;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,' ');
	} else {
		if(ABSame && !CDSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelC_r,pixelC_g,pixelC_b,pixelA_r,pixelA_g,pixelA_b,"\u2580");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▀');
			return;
		}
		if(CDSame && !ABSame && !ACSame && !BDSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,"\u2584");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▄');
			return;
		}
		if(ACSame && !BDSame && !ABSame && !CDSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelB_r,pixelB_g,pixelB_b,pixelC_r,pixelC_g,pixelC_b,"\u258C");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▌');
			return;
		}
		if(BDSame && !ACSame && !ABSame && !CDSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelA_r,pixelA_g,pixelA_b,pixelB_r,pixelB_g,pixelB_b,"\u2590");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▐');
			return;
		}
		if(ACSame && CDSame && !ABSame && !BCSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelB_r,pixelB_g,pixelB_b,pixelC_r,pixelC_g,pixelC_b,"\u2599");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▙');
			return;
		}
		if(ABSame && BDSame && !ACSame && !CDSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelC_r,pixelC_g,pixelC_b,pixelD_r,pixelD_g,pixelD_b,"\u259C");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▜');
			return;
		}
		if(CDSame && BDSame && !ACSame && !ABSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,"\u259F");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▟');
			return;
		}
		if(ABSame && ACSame && !BDSame && !CDSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelD_r,pixelD_g,pixelD_b,pixelC_r,pixelC_g,pixelC_b,"\u259B");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▛');
			return;
		}
		if(ADSame && !ABSame && !BDSame && !BCSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelB_r,pixelB_g,pixelB_b,pixelA_r,pixelA_g,pixelA_b,"\u259A");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▚');
			return;
		}
		if(BCSame && !ABSame && !CDSame && !ACSame) {
			printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%s",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,"\u259E");
			// printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,pixelC_r,pixelC_g,pixelC_b,'▞');
			return;
		}
		
		printf("\033[48;2;%d;%d;%dm%c",pixelA_r,pixelA_g,pixelA_b,' ');
		// printf("\033[0m%c",' ');
		cerr << "Unkown: " << ABSame << BCSame << CDSame << ADSame << ACSame << BDSame << endl;
	}
	// printf("\033[0m%c",' ');
	// return "\033[0m "

	// printf("\033[48;2;%d;%d;%dm%c%c",r,g,b,' ');
	// printf("\033[38;2;%d;%d;%dm%c%c",r,g,b,' ');
}

void draw_frame(Mat frame,int termWitdh,int termHeight) {
	int channels = frame.channels();
	uint8_t* pixelPtr = (uint8_t*)frame.data;

	#ifndef USE_UNICODE_FOR_BETTER_GRAPHICS
		for (int y = 0; y < termHeight-2; y++) {
			for (int x = 0; x < termWitdh; x++) {
				int b = pixelPtr[x*frame.cols*channels + y*channels + 0]; // B
	        	int g = pixelPtr[x*frame.cols*channels + y*channels + 1]; // G
	        	int r = pixelPtr[x*frame.cols*channels + y*channels + 2]; // R
				printf("\033[48;2;%d;%d;%dm ",r,g,b);
			}
			if(y != termHeight-2) {cout << "\n";}
		}
	#else
		for (int y = 0; y < termHeight-2; y+=2) {
			for (int x = 0; x < termWitdh; x+=2) {
				if((x+1) > termWitdh) { x= termWitdh; }
				if((y+1) > (termHeight-2)) { x= termHeight-2; }
				draw_pixel(frame,pixelPtr,x,y);
			}
			if(y != termHeight-2) {cout << "\n";}
		}
	#endif
	// printf("\033[48;2;%d;%d;%dm%c ",255,0,0);
	printf("\033[0;0;H");
	printf("\033[48;2;0;0;0m");
	printf("\033[38;2;0;0;0m");
}


int main(int argc, char* argv[])
{

	if (argc<2) {
		cerr << "Usage: " << argv[0] << " input.mp4 " << endl;
		cerr << "       " << argv[0] << " /dev/video0 " << endl;
		return -1;
	}

	VideoCapture cap(argv[1]);
	if (cap.isOpened() == false) {
		cerr << "Can!open the video file" << endl;
		return -1;
	}

	double fps = cap.get(CAP_PROP_FPS); 
	int actualFps = 0.0;
	int sWidth = cap.get(CAP_PROP_FRAME_WIDTH);
	int sHeight = cap.get(CAP_PROP_FRAME_HEIGHT);
	long frameCounter = 0;

	double fpsDelay = (1.0/fps) *1000*1000;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	
	while (true) {
		clock_t tStart = clock();


		Mat frame;
		Mat frameResized;
		usleep(fpsDelay);
		bool bSuccess = cap.read(frame);
		if(bSuccess) {
			frameCounter++;


			cerr << "Video size: " << sWidth << "x" << sHeight << " Term size: " << size.ws_row << "x" << size.ws_col  << " Set rate: " << fps << " Actual rate: " << actualFps << " Frame no: " << frameCounter << endl;

			frame = rotate_frame(frame,90);
			#ifdef USE_UNICODE_FOR_BETTER_GRAPHICS
    			// blur(frame,frame,Size(30,30)); 
				cv::resize(frame, frameResized, cv::Size(size.ws_row*2,size.ws_col*2), 0, 0, INTER_LINEAR);
				draw_frame(frameResized,size.ws_col*2, size.ws_row*2);
			#else
				cv::resize(frame, frameResized, cv::Size(size.ws_row,size.ws_col), 0, 0, INTER_LINEAR);
				draw_frame(frameResized,size.ws_col, size.ws_row);
			#endif
		} else {
			cout << "\033[0m \x1bc";
			cerr << "End of stream" << endl;
			break;
		}

		actualFps = 1 / ((double)(clock() - tStart)/CLOCKS_PER_SEC);
	}
	return 0;
}

