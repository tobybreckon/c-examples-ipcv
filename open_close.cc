// Example : open and close an image / video / camera
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

  IplImage* img;  			 		// input image object
  IplImage* openImage = NULL;  		// open image object
  IplImage* closeImage = NULL;      // close image object

  CvCapture* capture = NULL; // capture object

  char* windowName1 = "OPENCV: open"; // window name
  char* windowName2 = "OPENCV: close"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  IplConvKernel* structuringElement; // open/close structuring element
  int rows, columns;				 // structuring element rows/columns
  int iterations;					 // iterations to apply

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( CAMERA_INDEX )) != 0 )
	  )
    {
      // create window objects

      cvNamedWindow(windowName1, 0 );
      cvNamedWindow(windowName2, 0 );
	  cvNamedWindow("controls", 0 );

	  // init. user controls

	  rows = 1;
	  columns = 1;
	  iterations = 1;
	  cvCreateTrackbar("Element rows", "controls", &rows, 15, NULL);
	  cvCreateTrackbar("Element columns", "controls", &columns, 15, NULL);
	  cvCreateTrackbar("iterations", "controls", &iterations, 25, NULL);


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

	  openImage = cvCreateImage(cvSize(img->width,img->height),
						   img->depth, img->nChannels);
	  openImage->origin = img->origin;
	  closeImage = cvCreateImage(cvSize(img->width,img->height),
				img->depth, img->nChannels);
	  closeImage->origin = img->origin;

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

		  // do open / close with variable size structuring element


		  if (rows < 1){rows++;}
		  if (columns < 1) {columns++;}
		  if (iterations < 1) {iterations++;}
		  structuringElement = cvCreateStructuringElementEx(rows, columns,
		  		cvFloor(rows / 2), cvFloor(columns / 2), CV_SHAPE_RECT, NULL);


		  cvMorphologyEx(img, openImage, NULL, structuringElement,
		  											CV_MOP_OPEN, iterations);

		  cvMorphologyEx(img, closeImage, NULL, structuringElement,
		  											CV_MOP_CLOSE, iterations);

		  cvReleaseStructuringElement(&structuringElement);

		  // display images in window

		  cvShowImage( windowName1, openImage );
		  cvShowImage( windowName2, closeImage );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 4 ms roughly equates to 100ms/25fps = 4ms per frame

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

	  // destroy image objects

      cvReleaseImage( &openImage );
	  cvReleaseImage( &closeImage );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
