// Example : unsharp transform of image / video / camera
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>
#include <algorithm> // contains max() function (amongst others)
using namespace cv; // use c++ namespace so the timing stuff works consistently

/******************************************************************************/
// setup the camera index properly based on OS platform

// 0 in linux gives first camera for v4l
//-1 in windows gives first device or user dialog selection

#ifdef linux
	#define CAMERA_INDEX 0
#else
	#define CAMERA_INDEX -1
#endif

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object
  CvCapture* capture = NULL; // capture object

  char const * windowName1 = "OpenCV: unsharp transform "; // window name
  char const * windowName2 = "Original Image"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  double k = 0.1;				// parameters
  int k_mult = 1;
  int windowSize = 3;


  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( 0 )) != 0 )
	  )
    {
      // create window object (use flag=0 to allow resize, 1 to auto fix size)

      cvNamedWindow(windowName1, 0);
	  cvNamedWindow(windowName2, 0);

	  // add adjustable trackbar for parameter

	  cvCreateTrackbar("k (* 0.1)", windowName1, &k_mult, 100, NULL);
	  cvCreateTrackbar("smooth NxN", windowName1, &windowSize, 10, NULL);

	  // define required images for intermediate processing
      // (if using a capture object we need to get a frame first to get the size)

	  if (capture) {

		  // cvQueryFrame s just a combination of cvGrabFrame
		  // and cvRetrieveFrame in one call.

		  img = cvQueryFrame(capture);
		  if(!img){
			if (argc == 2){
				printf("End of video file reached\n");
			} else {
				printf("ERROR: cannot get next fram from camera\n");
			}
			exit(0);
		  }

	  }

	  // create required images

	  IplImage* grayImg =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  grayImg->origin = img->origin;

	  IplImage* smooth =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  smooth->origin = img->origin;

	  IplImage* subtracted =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  subtracted->origin = img->origin;

	  IplImage* unsharp =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  unsharp->origin = img->origin;

	  // start main loop

	  while (keepProcessing) {

          int64 timeStart = getTickCount(); // get time at start of loop

		  // if capture object in use (i.e. video/camera)
		  // get image from capture object

		  if (capture) {

			  // cvQueryFrame s just a combination of cvGrabFrame
			  // and cvRetrieveFrame in one call.

			  img = cvQueryFrame(capture);
			  if(!img){
				if (argc == 2){
					printf("End of video file reached\n");
				} else {
					printf("ERROR: cannot get next fram from camera\n");
				}
				exit(0);
			  }

		  }


		  // if input is not already grayscale, convert to grayscale

			  if (img->nChannels > 1){
				  cvCvtColor(img, grayImg, CV_BGR2GRAY);
			  } else {
				  grayImg = img;
			  }

		  // check that the window size is always odd and > 3

		  if ((windowSize > 3) && (fmod((double) windowSize, 2) == 0)) {
				windowSize++;
		  } else if (windowSize < 3) {
			  windowSize = 3;
		  }

		  // do the unsharp(as per algorithm in lecture 9)

		  cvSmooth(grayImg, smooth, CV_BLUR, windowSize, windowSize, 0, 0);
          cvSub(grayImg, smooth, subtracted, NULL);
		  cvAddWeighted(grayImg, 1, subtracted, (k * k_mult), 0, unsharp);

		  // display images in window

		  cvShowImage( windowName1, unsharp );
		  cvShowImage( windowName2, grayImg );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 40 ms roughly equates to 1000ms/25fps = 4ms per frame

		  // here we take account of processing time for the loop by subtracting the time
          // taken in ms. from this (1000ms/25fps = 40ms per frame) value whilst ensuring
          // we get a +ve wieght time

          key = waitKey((int) std::max(2.0, EVENT_LOOP_DELAY -
                        (((getTickCount() - timeStart) / getTickFrequency()) * 1000)));

		  if (key == 'x'){

	   		// if user presses "x" then exit

	   			printf("Keyboard exit requested : exiting now - bye!\n");
	   			keepProcessing = false;
		  }
	  }

      // destroy window objects
      // (triggered by event loop *only* window is closed)

      cvDestroyAllWindows();

      // destroy image objects (if it does not originate from a capture object)

      if (!capture){
		  cvReleaseImage( &img );
      }
	  cvReleaseImage(&grayImg);
	  cvReleaseImage(&subtracted);
	  cvReleaseImage(&smooth);
	  cvReleaseImage(&unsharp);

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
