// Example : hough lines of image / video / camera
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
	#define CAMERA_INDEX 1
#else
	#define CAMERA_INDEX -1
#endif

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object
  CvCapture* capture = NULL; // capture object

  char* windowName = "Hough Circle Detection"; // window name
  char* controls = "Controls"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame
  int upperThreshold = 200; // upper canny edge threshold initial setting

  int minDistance = 100; 		// distance parameter for hough
  int houghThreshold = 70;
  int minRadius = 25;
  int maxRadius = 100;

  CvSeq* circles = NULL;

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], CV_LOAD_IMAGE_UNCHANGED)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( CAMERA_INDEX )) != 0 )
	  )
    {
      // create window object (use flag=0 to allow resize, 1 to auto fix size)

      cvNamedWindow(windowName, 0);
	  cvNamedWindow(controls, 0);
 	  cvCreateTrackbar("Upper", controls, &upperThreshold, 255, NULL);
	  cvCreateTrackbar("Min. Distance", controls, &minDistance, 200, NULL);
      cvCreateTrackbar("threshold", controls, &houghThreshold, 255, NULL);
	  cvCreateTrackbar("Min. Radius", controls, &minRadius, 100, NULL);
      cvCreateTrackbar("Max. Radius", controls, &maxRadius, 255, NULL);

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

	  // define working images

	  IplImage* gray_dst = cvCreateImage( cvGetSize(img), img->depth, 1);
	  gray_dst->origin = img->origin;
	  CvMemStorage* storage = cvCreateMemStorage(0);

	  // start main loop

	  while (keepProcessing) {

          int64 timeStart = getTickCount(); // get time at start of loop

		  // if capture object in use (i.e. video/camera)
		  // get image from capture object

		  if (capture) {

			  // cvQueryFrame is just a combination of cvGrabFrame
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

		  }	else {

			  // if not a capture object set event delay to zero so it waits
			  // indefinitely (as single image file, no need to loop)

			  EVENT_LOOP_DELAY = 0;
		  }

		  // compute gray image

		  cvCvtColor( img, gray_dst, CV_BGR2GRAY );

		  // get hough lines

		  if (minDistance == 0){minDistance++;}
		  if (upperThreshold == 0){upperThreshold++;}
		  if (houghThreshold == 0){houghThreshold++;}

		  circles = cvHoughCircles( gray_dst, storage, CV_HOUGH_GRADIENT,
		  			2, minDistance, upperThreshold, houghThreshold,
		       		minRadius, maxRadius);

		  // draw hough lines

			for(int i = 0; i < circles->total; i++ )
			{
				 float* p = (float*)cvGetSeqElem( circles, i );
				 cvCircle( img, cvPoint(cvRound(p[0]),cvRound(p[1])), 2, CV_RGB(0,255,0), -1, 8, 0 );
				 cvCircle( img, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 2, 8, 0 );
			}
		    cvClearSeq(circles);

		  // display image in window

		  cvShowImage( windowName, img );

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

      // destroy image object (if it does not originate from a capture object)

      if (!capture){
		  cvReleaseImage( &img );
      }
      cvReleaseImage( &gray_dst );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
