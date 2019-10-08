#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

struct winsize size;
using namespace cv;
using namespace std;

Mat rotate_frame(Mat src,double angle) {

    // get rotation matrix for rotating the image around its center in pixel coordinates
    cv::Point2f center((src.cols-1)/2.0, (src.rows-1)/2.0);
    cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
    // determine bounding rectangle, center not relevant
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
    // adjust transformation matrix
    rot.at<double>(0,2) += bbox.width/2.0 - src.cols/2.0;
    rot.at<double>(1,2) += bbox.height/2.0 - src.rows/2.0;

    cv::Mat dst;
    cv::warpAffine(src, dst, rot, bbox.size());
    return dst;
}

void draw_frame(Mat frame,int termWitdh,int termHeight) {
	uint8_t* pixelPtr = (uint8_t*)frame.data;
	int cn = frame.channels();

	for (int y = 0; y < termHeight-2; ++y) {
		for (int x = 0; x < termWitdh; ++x) {
			int b = pixelPtr[x*frame.cols*cn + y*cn + 0]; // B
        	int g = pixelPtr[x*frame.cols*cn + y*cn + 1]; // G
        	int r = pixelPtr[x*frame.cols*cn + y*cn + 2]; // R
			printf("\033[48;2;%d;%d;%dm ",r,g,b);
		}
		if(y != termHeight-2) {cout << "\n";}
	}
	// printf("\033[48;2;%d;%d;%dm ",255,0,0);
	printf("\033[0;0;H");
	printf("\033[48;2;0;0;0m");
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
		cerr << "Cannot open the video file" << endl;
		return -1;
	}

	double fps = cap.get(CAP_PROP_FPS); 
	int actualFps = 0.0;
	int sWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int sHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	long frameCounter = 0;

	double fpsDelay = (1.0/fps) *1000*1000;
	
	while (true) {
		clock_t tStart = clock();


		Mat frame;
		Mat frameResized;
		usleep(fpsDelay);
		bool bSuccess = cap.read(frame);
		if(bSuccess) {
			frameCounter++;

			ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

			cerr << "Video size: " << sWidth << "x" << sHeight << " Term size: " << size.ws_row << "x" << size.ws_col  << " Set rate: " << fps << " Actual rate: " << actualFps << " Frame no: " << frameCounter << endl;

			frame = rotate_frame(frame,90);
			cv::resize(frame, frameResized, cv::Size(size.ws_row,size.ws_col), 0, 0, CV_INTER_LINEAR);
			
			draw_frame(frameResized,size.ws_col, size.ws_row);

		} else {
			cout << "\033[0m \x1bc";
			cerr << "End of stream" << endl;
			break;
		}

		actualFps = 1 / ((double)(clock() - tStart)/CLOCKS_PER_SEC);
	}
	return 0;
}

