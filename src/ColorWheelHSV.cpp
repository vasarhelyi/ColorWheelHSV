// ColorWheelHSV v1.0: Display an interactive HSV color wheel using OpenCV's HSV color converter. by Shervin Emami (shervin.emami@gmail.com), 6th Nov 2009.
// Note that OpenCV's HSV color model is different to the more common HSV color models in graphics software.
// In OpenCV, Hue varies between 0 to 179 (not 0 to 359), and a Saturation of 255 is a bright color, whereas it typically white in other software.

// This version of the software is substantially rewritten by Gabor Vasarhelyi (vasarhelyi@hal.elte.hu).

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <cstdio>	// Used for "printf"
#include <string>	// Used for C++ strings
#include <iostream>	// Used for C++ cout print statements
#include <cmath>	// Used to calculate square-root for statistics

// Include OpenCV libraries
#include <opencv2/opencv.hpp>

#include "VersionNo.h"

using namespace std;

const int HUE_RANGE = 180;	// OpenCV just uses Hues between 0 to 179!

const int WIDTH = 361;		// Window size
const int HEIGHT = 306;		//		"
const int HUE_HEIGHT = 25;		// thickness of Hue chart
const int WHEEL_TOP = HUE_HEIGHT + 20;		// y position for top of color wheel (= Hue height + gap)
const int WHEEL_BOTTOM = WHEEL_TOP + 255;	// y position for bottom of color wheel
const int TILE_LEFT = 280;	// Position of small tile showing highlighted color
const int TILE_TOP = 140;	//		"
const int TILE_W = 60;		//		"
const int TILE_H = 60;		//		"

const char *windowMain = "HSV Color Wheel. Click a color, or press ESC to quit";	// title of the window
const char *windowHSVFilter = "Filtered Image. Change HSV Color and Range and check result."; // title of the HSV filter window

// a particle (chip/bin or total ID)
class cColor {
public:
	int H;
	int S;
	int V;
	int rangeH;
	int rangeS;
	int rangeV;
	//! Constructor.
	cColor()
		:H(90),S(240),V(200)
		,rangeH(10),rangeS(50),rangeV(50)
	{}
	// overloading 'Equal To' operator
	bool operator==(const cColor &other) const {
		if (this->H == other.H &&
			this->S == other.S &&
			this->V == other.V &&
			this->rangeH == other.rangeH &&
			this->rangeS == other.rangeS &&
			this->rangeV == other.rangeV)
			return true;
		else
			return false;
	}
	// overloading 'Not Equal To' operator
	bool operator!=(const cColor &other) const {
		return !(*this == other);
	}
};

cColor color;
std::vector<cColor> colorvec; // to draw all saved colors to see how they are organized
std::vector<cColor> colorhistory; // to be able to have longer undo

int mouseX = -1;	// Position in the window that a user clicked the mouse button.
int mouseY = -1;	//		"

int avgpixnum = 1; // how many pixels to average on right mouse click?
int avgcolornum = 0; // how many colors have been selected with Shift-Left mouse to average color?

char inputfile[256]; // inputfile is read as first command line argument
int currentframe = 0, currentframe2 = 0; // which frame to read first?
int framecount = 0; // how many frames are there?
const double FILTERIMAGEDISPLAYWIDTH = 500; // display width
cv::Mat inputimage; // original image
cv::VideoCapture inputvideo; // video
bool bInputIsImage = false;
int iDrawBlobs = 0;

int iHighlightChannel = 3; // 0 = blue, 1 = green, 2 = red, 3 = white

void DrawBlob(cv::Mat &dstImg, cv::Moments& moments) {
    int dia, centerx, centery;
    // draw circles around blobs
    dia = (int)(sqrt(moments.m00 / 3.14159265) * 2);
    centerx = (int)(moments.m10 / moments.m00);
    centery = (int)(moments.m01 / moments.m00);
    cv::circle(dstImg, cv::Point(centerx, centery), dia, cv::Scalar(0, 0, 255), 2);
    // write out area
    char c[32];
    snprintf(c, 32, "A:%d, D:%d", int(moments.m00), dia);
    cv::putText(dstImg, c, cv::Point(centerx + dia + 5, centery), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
}

void DrawBlobs(cv::Mat &srcBin, cv::Mat &dstImg, cColor* mColor, int iDrawBlobs) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::Moments moments, largestmoments;
    unsigned int j = 0;
    double maxarea = 0;

    // do not draw anything on 0
    if (iDrawBlobs == 0) {
        return;
    }

    // find blob contours
    cv::findContours(srcBin, contours, hierarchy, cv::RETR_EXTERNAL,
        cv::CHAIN_APPROX_NONE, cv::Point(0, 0));

    // Iterate over blobs
    for (j = 0; j < contours.size(); j++) {
        // Compute the moments
        moments = cv::moments(contours[j]);
        // draw all on 2
        if (iDrawBlobs == 2) {
            DrawBlob(dstImg, moments);
        }
        else if (iDrawBlobs == 1 && moments.m00 > maxarea) {
            maxarea = moments.m00;
            largestmoments = moments;
        }
    }

    if (maxarea > 0) {
        DrawBlob(dstImg, largestmoments);
    }
}


////////////////////////////////////////////////////////////////////////////////
// source: http://www.shervinemami.info/blobs.html
// input file type must be HSV 8-bit
// output file type must be same size, binary 8-bit
void cvFilterHSV(cv::Mat &dstBin, cv::Mat &srcHSV) {
	int Hmin,Hmax,Smin,Smax,Vmin,Vmax,x;

	// Hue: 0-180, circular continuous
	Hmin = Hmax = color.H;
	x = color.rangeH; if (x>89) x = 89;
	Hmax = (Hmax+x)%180;
	Hmin = (Hmin+180-x)%180;

	// Saturation: 0-255
	Smin = Smax = color.S;
	x = color.rangeS;
	Smax += x; if (Smax>255) Smax = 255;
	Smin -= x; if (Smin<0) Smin = 0;

	// Value: 0-255
	Vmin = Vmax = color.V;
	x = color.rangeV;
	Vmax += x; if (Vmax>255) Vmax = 255;
	Vmin -= x; if (Vmin<0) Vmin = 0;

	// threshold H plane
	if (Hmax >= Hmin)
		cv::inRange(srcHSV, cv::Scalar(Hmin,Smin,Vmin), cv::Scalar(Hmax,Smax,Vmax), dstBin);
	else
	{
        cv::Mat tmp;
		cv::inRange(srcHSV, cv::Scalar(Hmin,Smin,Vmin), cv::Scalar(255,Smax,Vmax), dstBin);
		cv::inRange(srcHSV, cv::Scalar(0,Smin,Vmin), cv::Scalar(Hmax,Smax,Vmax), tmp);
		cv::bitwise_or(tmp, dstBin, dstBin);
	}
}

void displayFilteredImage() {
	// create copy image
    cv::Mat filterimage, outputimage;
	// covert to HSV
	cv::cvtColor(inputimage, outputimage, cv::COLOR_BGR2HSV);
	// filter it
    cvFilterHSV(filterimage, outputimage);

    // convert binary to RGB
    std::vector<cv::Mat> images(3);

    // white
    if (iHighlightChannel > 2) {
        images.at(0) = filterimage;
        images.at(1) = filterimage;
        images.at(2) = filterimage;
        cv::merge(images, outputimage);
        cv::bitwise_or(inputimage, outputimage, outputimage);
    }
    // blue, green, red
    else {
        cv::split(inputimage, images);
        cv::bitwise_or(images.at(iHighlightChannel), filterimage, images.at(iHighlightChannel));
        cv::bitwise_and(images.at((iHighlightChannel + 1) % 3), 255 - filterimage, images.at((iHighlightChannel + 1) % 3));
        cv::bitwise_and(images.at((iHighlightChannel + 2) % 3), 255 - filterimage, images.at((iHighlightChannel + 2) % 3));
        cv::merge(images, outputimage);
    }

	// draw blobs on it
    DrawBlobs(filterimage, outputimage, &color, iDrawBlobs);
    // show it
	cv::imshow(windowHSVFilter, outputimage);
}

int getNewFramesFromVideo(int n=1) {
	cv::Mat tempimage;
	int i = 0;
	while (i<n) {
		if (framecount && currentframe && currentframe >= framecount) break;
        inputvideo.read(tempimage);
		if (tempimage.empty())
		{
			cout << "error reading new frame from video!" << endl;
			return i;
		}
		inputimage = tempimage;
		currentframe++;
		i++;
	}
    // gaussian smoothing of input image to reduce speckle/interlace noise
    if (i) {
        cv::GaussianBlur(inputimage, inputimage, cv::Size(3, 3), 0);
    }
	return i;
}

//void getImageFromVideo(int state, void* userdata) // used by cvCreateButtom
void getImageFromVideo(int pos, void *userdata) { // used by cvCreateTrackbar
//	if (currentframe >= framecount) return;
	// TODO bug: the line below had to be commented out because in our rat stream it does not work. Always reading the next frame...
	//cvSetCaptureProperty(inputvideo,CV_CAP_PROP_POS_FRAMES,currentframe);
//	getNewFramesFromVideo();
	displayFilteredImage();
	currentframe2 = currentframe;
}

void displayColorWheelHSV(void) {
	static cColor oldcolor;
	cv::Mat imageHSV(cv::Size(WIDTH, HEIGHT), CV_8UC3);
	int rowSize = (int)imageHSV.step;	// Size of row in bytes, including extra padding
	unsigned char *imOfs = imageHSV.data;	// Pointer to the start of the image HSV pixels.

	// Clear the image to grey (Saturation=0)
	imageHSV.setTo(cv::Scalar(0,0,210, 0));

	// Draw the hue chart on the top, at double width.
	for (int y=0; y<HUE_HEIGHT; y++) {
		for (int x=0; x<HUE_RANGE; x++) {
			uchar h = x;		// Hue (0 - 179)
			uchar s = 255;		// max Saturation => most colorful
			uchar v = 255;		// max Value => brightest color
			// Highlight the current value
			if ((h == color.H-2 || h == color.H+2) && (y < HUE_HEIGHT/2)) {
				s = 0;	// make it white instead of the color
			}
			// Set the HSV pixel components
			*(uchar*)(imOfs + y*rowSize + (x*2+0)*3 + 0) = h;
			*(uchar*)(imOfs + y*rowSize + (x*2+0)*3 + 1) = s;
			*(uchar*)(imOfs + y*rowSize + (x*2+0)*3 + 2) = v;
			*(uchar*)(imOfs + y*rowSize + (x*2+1)*3 + 0) = h;
			*(uchar*)(imOfs + y*rowSize + (x*2+1)*3 + 1) = s;
			*(uchar*)(imOfs + y*rowSize + (x*2+1)*3 + 2) = v;
		}
	}

	// Draw the color wheel: Saturation on the x-axis and Value (brightness) on the y-axis.
	for (int y=0; y<255; y++) {
		for (int x=0; x<255; x++) {
			uchar h = color.H;		// Hue (0 - 179)
			uchar s = x;		// Saturation (0 - 255)
			uchar v = (255-y);	// Value (Brightness) (0 - 255)
			// Highlight the current value
			if ((s == color.S-2 || s == color.S-3 || s == color.S+2 || s == color.S+3) && (v == color.V-2 || v == color.V-3 || v == color.V+2 || v == color.V+3)) {
				s = 0;	// make it white instead of the color
				v = 0;	// bright white
			}
			// Set the HSV pixel components
			*(uchar*)(imOfs + (y+WHEEL_TOP)*rowSize + x*3 + 0) = h;
			*(uchar*)(imOfs + (y+WHEEL_TOP)*rowSize + x*3 + 1) = s;
			*(uchar*)(imOfs + (y+WHEEL_TOP)*rowSize + x*3 + 2) = v;
		}
	}

	// highlight the saved colors
	for (std::vector<cColor>::iterator it = colorvec.begin(); it<colorvec.end(); ++it) {
		// H
		cv::rectangle(imageHSV,
			cv::Point((*it).H*2-(*it).rangeH,1),
			cv::Point((*it).H*2+(*it).rangeH,HUE_HEIGHT-1),
			cv::Scalar(0,0,0),1);
		// S + V
		cv::rectangle(imageHSV,
			cv::Point((*it).S-(*it).rangeS/2,WHEEL_TOP+255-(*it).V-(*it).rangeV/2),
			cv::Point((*it).S+(*it).rangeS/2,WHEEL_TOP+255-(*it).V+(*it).rangeV/2),
			cv::Scalar((*it).H,255,255),1);
	}

	// Draw a small tile of the highlighted color.
	for (int y=0; y<TILE_H; y++) {
		for (int x=0; x<TILE_W; x++) {
			// Set the HSV pixel components
			*(uchar*)(imOfs + (y+TILE_TOP)*rowSize + (x+TILE_LEFT)*3 + 0) = color.H;
			*(uchar*)(imOfs + (y+TILE_TOP)*rowSize + (x+TILE_LEFT)*3 + 1) = color.S;
			*(uchar*)(imOfs + (y+TILE_TOP)*rowSize + (x+TILE_LEFT)*3 + 2) = color.V;
		}
	}

	// Convert the HSV image to RGB (BGR) for displaying
	cv::Mat imageRGB(cv::Size(imageHSV.cols, imageHSV.rows), CV_8UC3);
	cv::cvtColor(imageHSV, imageRGB, cv::COLOR_HSV2BGR);	// (note that OpenCV stores RGB images in B,G,R order.

	// Display the RGB image
	cv::imshow(windowMain, imageRGB);

	// write text to output
	if (oldcolor != color) {
		cout	<< "HSV: " << color.H << " " << color.S << " " << color.V
				<< " rangeHSV: " << color.rangeH << " " << color.rangeS << " " << color.rangeV << endl;
		oldcolor = color;
	}
	// update HSV filter window
	displayFilteredImage();
}

// This function is automatically called whenever the user changes the trackbar value.
void color_trackbarWasChanged(int pos, void *userdata) {
	displayColorWheelHSV();
}

void undo(bool bUpdateGUI=true) {
    if (colorhistory.size() > 0) {
        color = colorhistory.back();
        colorhistory.pop_back();
        cout << "stepped back one color" << endl;
    }
    else {
        cout << "nothing to undo" << endl;
    }
    // TODO: this is not very clean how above and below should relate...
    if (avgpixnum > 1) {
        avgpixnum--;
    }
    if (avgcolornum) {
        avgcolornum--;
    }

    if (bUpdateGUI) {
        // update the GUI Trackbars
        cv::setTrackbarPos("Hue", windowMain, color.H);
        cv::setTrackbarPos("Saturation", windowMain, color.S);
        cv::setTrackbarPos("Brightness", windowMain, color.V);
        cv::setTrackbarPos("rangeH", windowMain, color.rangeH);
        cv::setTrackbarPos("rangeS", windowMain, color.rangeS);
        cv::setTrackbarPos("rangeV", windowMain, color.rangeV);
    }
}

// This function is automatically called whenever the user clicks the mouse in the window.
static void mouseEvent( int ievent, int x, int y, int flags, void* param ) {
	// Check if they clicked or dragged a mouse button or not.
	if (flags & cv::EVENT_FLAG_LBUTTON) {
		colorhistory.push_back(color); // save old color
		mouseX = x;
		mouseY = y;
		//cout << mouseX << "," << mouseY << endl;

		// If they clicked on the Hue chart, select the new hue.
		if (mouseY < HUE_HEIGHT) {
			if (mouseX/2 < HUE_RANGE) {	// Make sure its a valid Hue
				color.H = mouseX/2;
				cv::setTrackbarPos("Hue", windowMain, color.H);	// update the GUI Trackbar
				// Note that "cv::setTrackbarPos()" will implicitly call "displayColorWheelHSV()" for a changed hue.
				//displayColorWheelHSV();
			}
		}
		// If they clicked on the Color wheel, select the new value.
		else if (mouseY >= WHEEL_TOP && mouseY <= WHEEL_BOTTOM) {
			if (mouseX < 256) {	// Make sure its a valid Saturation & Value
				color.S = mouseX;
				color.V = 255 - (mouseY - WHEEL_TOP);
				cv::setTrackbarPos("Saturation", windowMain, color.S);	// update the GUI Trackbar
				cv::setTrackbarPos("Brightness", windowMain, color.V);	// update the GUI Trackbar
				// Note that "cv::setTrackbarPos()" will implicitly call "displayColorWheelHSV()" for saturation or brightness.
				//displayColorWheelHSV();
			}
		}
	}
}

// This function is automatically called whenever the user clicks the mouse in the HSV filter window.
void mouseEvent2( int ievent, int x, int y, int flags, void* param ) {
	// left mouse click
	if (flags & cv::EVENT_FLAG_LBUTTON) {
		// Ctrl+left button click: save color, Ctrl+Shift+left: clear all colors
		if (flags & cv::EVENT_FLAG_CTRLKEY)
		{
			if (flags & cv::EVENT_FLAG_SHIFTKEY) colorvec.clear(); // clear all saved colors
			else colorvec.push_back(color); // save current color
			// update the color windows
			displayColorWheelHSV();
		}
		// Shift+left button click: average colors
		else if (flags & cv::EVENT_FLAG_SHIFTKEY) {
            colorhistory.push_back(color); // save old color
			cv::Vec3d pixel;
			int i,j;
			// average (2*n+1)^2 pixel neighborhood
			int n = 2; // neighbour radius (1-->3, 2-->5 etc.)
			if (!x) { x++; } if (x == inputimage.cols) { x--; }
			if (!y) { y++; } if (y == inputimage.rows) { y--; }
			for (i=0;i<3;i++) pixel.val[i] = 0;
			for (i=-n;i<=n;i++)
			{
				for (j=-n;j<=n;j++)
				{
					pixel.val[0] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[0];
					pixel.val[1] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[1];
					pixel.val[2] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[2];
				}
			}
			n = (2*n+1)*(2*n+1);
			for (i=0;i<3;i++) pixel.val[i] /= n;
			// get pixel color in HSV format
			//Cv::Scalar pixel = cvGet2D(inputimage,x,y); // bugbugbug this does not work somehow... BUT why???
			cv::Mat tmp(cv::Size(1,1), CV_8UC3);
            tmp.at<cv::Vec3b>(0, 0) = pixel;
			cv::cvtColor(tmp, tmp, cv::COLOR_BGR2HSV);
            pixel = tmp.at<cv::Vec3b>(0, 0);
			// set new average color
			color.H = (color.H * avgcolornum + (int)pixel.val[0]) / (avgcolornum + 1);
			color.S = (color.S * avgcolornum + (int)pixel.val[1]) / (avgcolornum + 1);
			color.V = (color.V * avgcolornum + (int)pixel.val[2]) / (avgcolornum + 1);
			// increase own, counter
			avgcolornum++;
			avgpixnum = 1; // reset counter to current selection
			// update the GUI Trackbars
			cout << avgcolornum << " colors averaged" << endl;
			cv::setTrackbarPos("Hue", windowMain, color.H);
			cv::setTrackbarPos("Saturation", windowMain, color.S);
			cv::setTrackbarPos("Brightness", windowMain, color.V);
		}
		// left mouse click: set values to pixel color (3x3 neighbour avg)
		else {
            colorhistory.push_back(color); // save old color
			avgpixnum = 1; // reset counter to current selection
			avgcolornum = 0; // reset counter
			cv::Vec3d pixel;
			int i,j;
			// average (2*n+1)^2 pixel neighborhood
			int n = 2; // neighbour radius (1-->3, 2-->5 etc.)
			if (!x) { x++; } if (x == inputimage.cols) { x--; }
			if (!y) { y++; } if (y == inputimage.rows) { y--; }
			for (i=0;i<3;i++) pixel.val[i] = 0;
			for (i=-n;i<=n;i++) {
				for (j=-n;j<=n;j++) {
					pixel.val[0] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[0];
					pixel.val[1] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[1];
					pixel.val[2] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[2];
				}
			}
			n = (2*n+1)*(2*n+1);
			for (i=0;i<3;i++) pixel.val[i] /= n;
			// get pixel color in HSV format
			cv::Mat tmp(cv::Size(1,1), CV_8UC3);
            tmp.at<cv::Vec3b>(0, 0) = pixel;
			cv::cvtColor(tmp, tmp, cv::COLOR_BGR2HSV);
            pixel = tmp.at<cv::Vec3b>(0, 0);
			// store new value
			color.H = (int)pixel.val[0];
			color.S = (int)pixel.val[1];
			color.V = (int)pixel.val[2];
			// update the GUI Trackbars
			cv::setTrackbarPos("Hue", windowMain, color.H);
			cv::setTrackbarPos("Saturation", windowMain, color.S);
			cv::setTrackbarPos("Brightness", windowMain, color.V);
		}
	}
	// right button
    else if (flags & cv::EVENT_FLAG_RBUTTON) {
        // Ctrl+right or Shift+right button click: restore last color / undo
        if (((flags & cv::EVENT_FLAG_CTRLKEY) > 0) != ((flags & cv::EVENT_FLAG_SHIFTKEY) > 0)) {
            undo(false);
        }
		// right button: adjust range exactly to fit all that are pointed
		else {
            colorhistory.push_back(color); // save old color
			cv::Vec3d pixel;
			int i,j;
			// average (2*n+1)^2 pixel neighborhood
			int n = 2; // neighbour radius (1-->3, 2-->5 etc.)
			if (!x) { x++; } if (x == inputimage.cols) { x--; }
			if (!y) { y++; } if (y == inputimage.rows) { y--; }
			for (i=0;i<3;i++) pixel.val[i] = 0;
			for (i=-n;i<=n;i++) {
				for (j=-n;j<=n;j++) {
					pixel.val[0] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[0];
					pixel.val[1] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[1];
					pixel.val[2] += inputimage.at<cv::Vec3b>(cv::Point(x + i, y + j))[2];
				}
			}
			n = (2*n+1)*(2*n+1);
			for (i=0;i<3;i++) pixel.val[i] /= n;
			// get pixel color in HSV format
			//CvScalar pixel = cvGet2D(inputimage,x,y); // bugbugbug this does not work somehow... BUT why???
			cv::Mat tmp(cv::Size(1,1),CV_8UC3);
            tmp.at<cv::Vec3b>(0, 0) = pixel;
			cv::cvtColor(tmp,tmp,cv::COLOR_BGR2HSV);
            pixel = tmp.at<cv::Vec3b>(0, 0);
			// if Ctrl+Shift+right button is pressed, reset tight inclusion
            if ((flags & cv::EVENT_FLAG_CTRLKEY) && (flags & cv::EVENT_FLAG_SHIFTKEY)) {
                avgpixnum = 0;
                avgcolornum = 0;
                cout << " averaging and multi-color inclusion is reset" << endl;
				color.rangeH = 1;
				color.rangeS = 1;
				color.rangeV = 1;
				color.H = (int)pixel.val[0];
				color.S = (int)pixel.val[1];
				color.V = (int)pixel.val[2];
			}
			// otherwise, include new color in the ranges
			else {
				////////////////// HUE ///////////////////
				// get hue range
				int Hmax,Hmin;
				Hmax = color.H; Hmin = Hmax;
				i = color.rangeH; if (i>89) i = 89;
				Hmax = (Hmax+i)%180;
				Hmin = (Hmin+180-i)%180;
				// if change is needed
				if ((Hmax>=Hmin && (pixel.val[0]>Hmax || pixel.val[0]<Hmin)) ||
					(Hmax<Hmin && (pixel.val[0]>Hmax && pixel.val[0]<Hmin)))
				{
					// i,j: distance from Hmax,Hmin
					i = ((int)pixel.val[0]+180-Hmax)%180;
					j = (Hmin+180-(int)pixel.val[0])%180;

					if (i<=j) Hmax = (int)pixel.val[0]; // closer to Hmax
					else Hmin = (int)pixel.val[0]; // closer to Hmin
					color.rangeH = (int)ceil((double)((Hmax+180-Hmin)%180)/2);
					color.H = (Hmin + color.rangeH)%180;
				}

				////////////////// SAT ///////////////////
				// maxS
				if (pixel.val[1]>color.S+color.rangeS) {
					i = max(0,color.S-color.rangeS);
					color.rangeS = (int)ceil((pixel.val[1]-i)/2);
					color.S = i + color.rangeS;
				}
				// minS
				else if (pixel.val[1]<color.S-color.rangeS) {
					i = min(255,color.S+color.rangeS);
					color.rangeS = (int)ceil((-pixel.val[1]+i)/2);
					color.S = (int)pixel.val[1] + color.rangeS;
				}

				////////////////// VAL ///////////////////
				// maxV
				if (pixel.val[2]>color.V+color.rangeV) {
					i = max(0,color.V-color.rangeV);
					color.rangeV = (int)ceil((pixel.val[2]-i)/2);
					color.V = i + color.rangeV;
				}
				// minV
				else if (pixel.val[2]<color.V-color.rangeV) {
					i = min(255,color.V+color.rangeV);
					color.rangeV = (int)ceil((-pixel.val[2]+i)/2);
					color.V = (int)pixel.val[2] + color.rangeV;
				}
			} // new range needed
			// increase own, reset other counter
			avgpixnum++;
			avgcolornum = 0;
			cout << avgpixnum << " colors included" << endl;
		} // right button pressed (no Ctrl)

		// update the GUI Trackbars
		cv::setTrackbarPos("Hue", windowMain, color.H);
        cv::setTrackbarPos("Saturation", windowMain, color.S);
        cv::setTrackbarPos("Brightness", windowMain, color.V);
        cv::setTrackbarPos("rangeH", windowMain, color.rangeH);
        cv::setTrackbarPos("rangeS", windowMain, color.rangeS);
        cv::setTrackbarPos("rangeV", windowMain, color.rangeV);
	} // right button pressed
}

// C++ entry point
int main(int argc, char **argv)
{
	cout << "HSV Color Wheel, by Shervin Emami (shervin.emami@gmail.com), 6th Nov 2009." << endl;
	cout << "HSVFiltering part added by Gabor Vasarhelyi (vasarhelyi@hal.elte.hu), since Jan 2011." << endl;
	cout << "Current version: " << VERSION_FILESTR << endl;
	cout << endl;
	cout << "Click on the top Hue map, or the bottom Color graph to change values." << endl;
	cout << endl;
	cout << "Mouse clicks on the image might help you as well:" << endl;
	cout << "  LEFT button: change values to 3x3 neighbor average color. Do not change range." << endl;
	cout << "  Shift+LEFT button: Average colors. Do not change range." << endl;
    cout << "  Ctrl+LEFT button: save current color and range and draw it on the palette." << endl;
    cout << "  Ctrl+Shift+LEFT button: clear all saved colors+ranges." << endl;
    cout << endl;
    cout << "  RIGHT button: include this pixel to new range as tight as possible." << endl;
    cout << "  Shift+RIGHT button: undo last mouseclick or console color input" << endl;
    cout << "  Ctrl+RIGHT button: undo last mouseclick or console color input" << endl;
    cout << "  Ctrl+Shift+RIGHT button: reset inclusion of multiple colors" << endl;
    cout << endl;
	cout << "Keyboard shortcuts (working only when the image window is the active one):" << endl;
	cout << "  n/N     - next frame" << endl;
	cout << "  f/F     - 100 frame forward" << endl;
	cout << "  h, s, v - start writing a number in the console, on Enter it will update color.H, .S, .V, respectively" << endl;
	cout << "  H, S, V - start writing a number in the console, on Enter it will update color.rangeH, .rangeS, .rangeV, respectively" << endl;
	cout << "  c/C     - start writing three numbers in the console with space between, on Enter it will update all color.H, .S, .V" << endl;
	cout << "  r/R     - start writing three numbers in the console with space between, on Enter it will update all color.rangeH, .rangeS, .rangeV" << endl;
    cout << "  d/D     - draw red circles and show area/diameter around none/largest/all blobs" << endl;
    cout << "  x/X     - change highlight color (blue, green, red, white)" << endl;
    cout << "  BackSpace - undo last color or range selection (of mouse clicks or console input)" << endl;
	cout << endl;

	if (argc > 2) {
		cout << "Please pvovide max 1 arg as input file name!" << endl;
		return -1;
	} else if (argc == 2) {
		strncpy(inputfile, argv[1], sizeof(inputfile));
	} else if (argc < 2) {
		cout << "Enter input file: ";
		cin >> inputfile;
	}
	cout << endl << "opening file " << inputfile << endl;
	// init input image
	if (!inputvideo.open(inputfile)) {
		cout << "error opening input video file, trying as image..." << endl;
		inputimage = cv::imread(inputfile);
		if (inputimage.empty()) {
			cout << "error opening input image file!" << endl;
			return -1;
		} else {
			bInputIsImage = true;
		}
	} else {
		bInputIsImage = false;
	}
	// get first 5 frames until something really appears from the stream
	currentframe = 0;
	if (!bInputIsImage) {
		int i = getNewFramesFromVideo(5);
		if (i == 0) {
			return -1;
		}
		if (i < 5) {
			bInputIsImage = true;
		}
	}

	// Create a GUI window
	cv::namedWindow(windowMain);
	// Allow the user to change the Hue value upto 179, since OpenCV uses Hues upto 180.
	cv::createTrackbar( "Hue", windowMain, &color.H, HUE_RANGE-1, &color_trackbarWasChanged);
	cv::createTrackbar( "Saturation", windowMain, &color.S, 255, &color_trackbarWasChanged);
	cv::createTrackbar( "Brightness", windowMain, &color.V, 255, &color_trackbarWasChanged);
	// Allow the user to click on Hue chart to change the hue, or click on the color wheel to see a value.
    cv::setMouseCallback( windowMain, mouseEvent);

	// TODO bug: sometimes first readout returns 0 in Win32. Why?
	// TODO bug: rat stream is buggy, framecount can be invalid
	if (!bInputIsImage) {
		framecount = (int)inputvideo.get(cv::CAP_PROP_FRAME_COUNT);
	} else {
		framecount = 1;
	}
	// TODO bug: max trackbar range in Win32 is 32767
	if (framecount>32768) framecount = 32768;
	cout << " framecount: " << framecount << endl;
	//inputvideo.set(CV_CAP_PROP_POS_FRAMES,currentframe);
	if (!bInputIsImage) {
		cv::createTrackbar( "frame", windowMain, &currentframe2, framecount-1, &getImageFromVideo );
	}
	// TODO bug: unreferenced external symbol cvCreateButton
	// solution: http://stackoverflow.com/questions/4458668/opencv-2-2-createbutton-lnk-2019-error-in-visual-studio-2010
	// second link: http://stackoverflow.com/questions/8904127/opencv-2-3-undefined-reference-to-cvcreatebutton-eclipse
	//cvCreateButton("frame",getImageFromVideo,NULL,CV_PUSH_BUTTON,0);

	// init HSV filter part
	cv::namedWindow(windowHSVFilter, cv::WINDOW_NORMAL);
	cv::resizeWindow(windowHSVFilter,(int)FILTERIMAGEDISPLAYWIDTH,(int)(inputimage.rows*FILTERIMAGEDISPLAYWIDTH/inputimage.cols));
	// Allow the user to change the Hue filter range value upto 179, since OpenCV uses Hues upto 180.
	cv::createTrackbar( "rangeH", windowMain, &color.rangeH, HUE_RANGE-1, &color_trackbarWasChanged);
	cv::createTrackbar( "rangeS", windowMain, &color.rangeS, 255, &color_trackbarWasChanged);
	cv::createTrackbar( "rangeV", windowMain, &color.rangeV, 255, &color_trackbarWasChanged);
	// Allow the user to click on input image to define basic HSV value
	cv::setMouseCallback( windowHSVFilter, mouseEvent2);

	// initialize display
	displayColorWheelHSV();

	// wait infinitely until Esc or Ctrl-C is pressed
	int a,b,c;
	int i = 0;
    int lasti = 0;
    int lastcommand = 0;
    int countdigits = 0;
    char digits[40];
	while (i != 27 && i != 3 && i != -1) {
        lasti = i;
        i = (cv::waitKey(0) & 255);
        if (!lastcommand) {
            if (!bInputIsImage) {
                // f, F
                if (i == 'f' || i == 'F') {
                    getNewFramesFromVideo(100);
                    cv::setTrackbarPos("frame", windowMain, currentframe);

                }
                // n, N
                else if (i == 'n' || i == 'N') {
                    getNewFramesFromVideo();
                    cv::setTrackbarPos("frame", windowMain, currentframe);
                }
            }
            // h, H, s, S, v, V
            if (i == 'h' || i == 'H' || i == 's' || i == 'S' || i == 'v' || i == 'V') {
                cout << "Please enter number for " << (char)i << ":" << endl;
                countdigits = 0;
                lastcommand = i;
            }
            // c, C
            else if (i == 'c' || i == 'C') {
                cout << "Please enter three numbers (and space between) for full HSV color definition:" << endl;
                countdigits = 0;
                lastcommand = 'c';
            }
            // r, R
            else if (i == 'r' || i == 'R') {
                cout << "Please enter three numbers (and space between) for full HSV range definition:" << endl;
                countdigits = 0;
                lastcommand = 'r';
            }
        }
        // digits and space (within lastcommand)
        else if (i == ' ' || (i >= '0' && i <= '9')) {
            digits[countdigits++] = (char)i;
            if (countdigits >= 40) {
                lastcommand = 0;
				cout << endl;
			}
			else {
				cout << (char)i;
			}
		}

        // BackSpace
        if (i == 8) {
            if (!lastcommand) {
                undo();
            } else if (countdigits) {
                countdigits--;
                cout << "\b \b";
            }
        }
        // Enter
        else if (i == 13) {
            cout << endl;
            if (lastcommand && countdigits) {
                digits[countdigits] = 0;
                if (lastcommand == 'h') {
                    colorhistory.push_back(color);
                    avgpixnum = 1;
                    color.H = atoi(digits);
                    cv::setTrackbarPos("Hue", windowMain, color.H);
                }
                else if (lastcommand == 'H') {
                    colorhistory.push_back(color);
                    avgpixnum = 1;
                    color.rangeH = atoi(digits);
                    cv::setTrackbarPos("rangeH", windowMain, color.rangeH);
                }
                else if (lastcommand == 's') {
                    colorhistory.push_back(color);
                    avgpixnum = 1;
                    color.S = atoi(digits);
                    cv::setTrackbarPos("Saturation", windowMain, color.S);
                }
                else if (lastcommand == 'S') {
                    colorhistory.push_back(color);
                    avgpixnum = 1;
                    color.rangeS = atoi(digits);
                    cv::setTrackbarPos("rangeS", windowMain, color.rangeS);
                }
                else if (lastcommand == 'v') {
                    colorhistory.push_back(color);
                    avgpixnum = 1;
                    color.V = atoi(digits);
                    cv::setTrackbarPos("Brightness", windowMain, color.V);
                }
                else if (lastcommand == 'V') {
                    colorhistory.push_back(color);
                    avgpixnum = 1;
                    color.rangeV = atoi(digits);
                    cv::setTrackbarPos("rangeV", windowMain, color.rangeV);
                }
                else if (lastcommand == 'c') {
                    if (sscanf(digits, "%d %d %d", &a, &b, &c) == 3) {
                        colorhistory.push_back(color);
                        avgpixnum = 1;
                        color.H = a;
                        color.S = b;
                        color.V = c;
                        cv::setTrackbarPos("Hue", windowMain, color.H);
                        cv::setTrackbarPos("Saturation", windowMain, color.S);
                        cv::setTrackbarPos("Brightness", windowMain, color.V);
                    } else {
                        cout << "invalid value, try again" << endl;
                    }
                }
                else if (lastcommand == 'r') {
                    if (sscanf(digits, "%d %d %d", &a, &b, &c) == 3) {
                        colorhistory.push_back(color);
                        avgpixnum = 1;
                        color.rangeH = a;
                        color.rangeS = b;
                        color.rangeV = c;
                        cv::setTrackbarPos("rangeH", windowMain, color.rangeH);
                        cv::setTrackbarPos("rangeS", windowMain, color.rangeS);
                        cv::setTrackbarPos("rangeV", windowMain, color.rangeV);
                    } else {
                        cout << "invalid value, try again" << endl;
                    }
                }
            }
            lastcommand = 0;
            countdigits = 0;
        }
        // draw blobs or not to draw blobs
        else if (i == 'd' || i == 'D') {
            iDrawBlobs = (iDrawBlobs + 1) % 3;
            displayFilteredImage();
        }
        // change highlight color
        else if (i == 'x' || i == 'X') {
            iHighlightChannel = (iHighlightChannel + 1) % 4;
            displayFilteredImage();
        }

        // anything else
        else if (!lastcommand) {
            if (i != lasti) {
                cout << "last pressed key: " << i << endl;
            }
        }
	}

	cv::destroyAllWindows();

	return 0;
}
