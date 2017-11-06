// Example : grab and display live video using canny edge detection
// usage: prog

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2007 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

// for OpenCV > 2.x use
//#include "opencv2/videoio.hpp"
//#include "opencv2/highgui.hpp"
//#include "opencv2/imgproc.hpp"

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>		// standard C++ I/O
#include <algorithm>    // includes max()

using namespace cv; // OpenCV API is in the C++ "cv" namespace
using namespace std;

int main( int argc, char** argv )
{

  IplImage* img;  // image object
  IplImage* grayImg = NULL;  // tmp image object
  IplImage* edgeImg = NULL;  // output image object

  int lowerThreshold = 50;  // lower canny edge threshold initial setting
  int upperThreshold = 200; // upper canny edge threshold initial setting
  int windowSize = 3;		// canny edge window size

  char const * windowName = "Durham University: Canny Edge Detection Demonstrator"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input

  int EVENT_LOOP_DELAY = 40; // timing of loop in ms

  // create window object

  cvNamedWindow(windowName, 0 );

  // add adjustable trackbars for ech of the canny edge parameters

  cvCreateTrackbar("Lower", windowName, &lowerThreshold, 255, NULL);
  cvCreateTrackbar("Upper", windowName, &upperThreshold, 255, NULL);
  cvCreateTrackbar("Window", windowName, &windowSize, 7, NULL);

  // grab an image from camera (here assume only 1 camera, device #0)

  CvCapture* capture = cvCaptureFromCAM(1); // capture from video device #0
  if(!cvGrabFrame(capture)){
    printf("Could not grab a frame\n");
    exit(0);
  }
  img=cvRetrieveFrame(capture); // retrieve the captured frame

  // create other images (and make sure they have the same origin)

  grayImg = cvCreateImage(cvSize(img->width,img->height),
				img->depth, 1);
  grayImg->origin = img->origin;

  edgeImg = cvCreateImage(cvSize(img->width,img->height),
				img->depth, 1);
  edgeImg->origin = img->origin;

  // grab the next frame to maintain sync

  cvGrabFrame(capture);

 // start main grab/process/display loop

  while (keepProcessing)
  {

   int64 timeStart = getTickCount(); // get time at start of loop

   img=cvRetrieveFrame(capture); // retrieve the captured frame

	// check that the window size is always odd and > 1

	if ((fmod((double) windowSize, 2) == 0) || (windowSize <= 2)) {
		windowSize++;
	}

   // convert colour of captured image to greyscale and perform
   // edge detection using canny edge detection algorithm

   cvCvtColor(img, grayImg, CV_BGR2GRAY);
   cvCanny(grayImg, edgeImg, lowerThreshold, upperThreshold, max(3, windowSize));

   // display image in window

   cvShowImage(windowName, edgeImg);

   // start event processing loop

   // here we use a 40 msec delay for ~25 fps (100/25 = 40)

   // here we take account of processing time for the loop by subtracting the time
   // taken in ms. from this (1000ms/25fps = 40ms per frame) value whilst ensuring
   // we get a +ve wait time


    key = cvWaitKey((int) std::max(2.0, EVENT_LOOP_DELAY -
                  (((getTickCount() - timeStart) / getTickFrequency()) * 1000)));

	if (key == 'x'){

	   // if user presses "x" then exit

	   printf("Keyboard exit requested : exiting now - bye!\n");
	   keepProcessing = false;
	}

	// capture next frame

    cvGrabFrame(capture);

  }

  // destroy window object
  // (triggered by event loop *only* window is closed)

  cvDestroyWindow( windowName );

  // release capture device

  cvReleaseCapture(&capture);

  // destroy image objects

  cvReleaseImage( &grayImg );
  cvReleaseImage( &edgeImg );

  // Note that the image captured by the device is allocated/released
  // by the capture function.
  // There is no need to release it explicitly.

  // all OK : main returns 0

  return 0;

}
