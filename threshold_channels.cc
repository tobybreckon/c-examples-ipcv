// Example : threshold R,G,B channels of an iput image source
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2007 School of Engineering, Cranfield University
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

  IplImage* img;  			 // input image object
  IplImage* redImg = NULL;  // red image channel
  IplImage* greenImg = NULL; // green image channel
  IplImage* blueImg = NULL;  // blue image channel
  IplImage* thresholdedImg = NULL;  // output image object

  char const * windowName1R = "OPENCV: RED Channel thresholding"; // window name
  char const * windowName1G = "OPENCV: GREEN Channel thresholding"; // window name
  char const * windowName1B = "OPENCV: BLUE Channel thresholding"; // window name
  char const * windowName2R = "OPENCV: RED Channel  image"; // window name
  char const * windowName2G = "OPENCV: GREEN Channel image"; // window name
  char const * windowName2B = "OPENCV: BLUE Channel image"; // window name

  int thresholdR = 150; // starting threshold value
  int thresholdG = 150; // starting threshold value
  int thresholdB = 150; // starting threshold value

  CvCapture* capture = NULL; // capture object

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( 0 )) != 0 )
	  )
    {

      // create window objects

      cvNamedWindow(windowName1B, 1 );
	  cvNamedWindow(windowName1G, 1 );
	  cvNamedWindow(windowName1R, 1 );
      cvNamedWindow(windowName2B, 1 );
	  cvNamedWindow(windowName2G, 1 );
	  cvNamedWindow(windowName2R, 1 );

	  // add adjustable trackbar for threshold parameters

      cvCreateTrackbar("Threshold", windowName1B, &thresholdB, 255, NULL);
	  cvCreateTrackbar("Threshold", windowName1G, &thresholdG, 255, NULL);
	  cvCreateTrackbar("Threshold", windowName1R, &thresholdR, 255, NULL);

	  // if capture object in use (i.e. video/camera)
	  // get initial image from capture object

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

	  // create output images

	  thresholdedImg = cvCreateImage(cvSize(img->width,img->height),
					   img->depth, 1);
      redImg = cvCreateImage(cvSize(img->width,img->height),
				img->depth, 1);
  	  redImg->origin = img->origin;
	  greenImg = cvCreateImage(cvSize(img->width,img->height),
	    		img->depth, 1);
  	  greenImg->origin = img->origin;
	  blueImg = cvCreateImage(cvSize(img->width,img->height),
				img->depth, 1);
  	  blueImg->origin = img->origin;

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

		  // if input is not already single channel, extract channels

	      if (img->nChannels > 1){
			cvSplit(img, blueImg, greenImg, redImg, NULL); // remember BGR ordering!

	     } else {
			redImg = greenImg = blueImg = img;
	     }

    	 // display images

	     cvShowImage( windowName2B, blueImg );
	     cvShowImage( windowName2G, greenImg );
	     cvShowImage( windowName2R, redImg );

		 // threshold each and display them

		 cvThreshold(blueImg, thresholdedImg, thresholdB, 255, CV_THRESH_BINARY);
		 cvShowImage( windowName1B, thresholdedImg );
		 cvThreshold(greenImg, thresholdedImg, thresholdG, 255, CV_THRESH_BINARY);
		 cvShowImage( windowName1G, thresholdedImg );
		 cvThreshold(redImg, thresholdedImg, thresholdR, 255, CV_THRESH_BINARY);
		 cvShowImage( windowName1R, thresholdedImg );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 4 ms roughly equates to 100ms/25fps = 4ms per frame

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
	  }

      // destroy window objects
      // (triggered by event loop *only* window is closed)

      cvDestroyAllWindows();

      // destroy image objects (if it does not originate from a capture object)

      if (!capture){
		  cvReleaseImage( &img );
      }
	  cvReleaseImage( &redImg );
	  cvReleaseImage( &blueImg );
	  cvReleaseImage( &greenImg );
	  cvReleaseImage( &thresholdedImg );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
