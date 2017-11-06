// Example : YCrCb channels display of image / video / camera
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

  IplImage* img = NULL;      // image object
  IplImage*	YCrCbimg = NULL;   // HAV image object
  CvCapture* capture = NULL; // capture object

  char const * windowName = "RGB Original Image"; 	// window name
  char const * windowNameY = "Y Channel"; 		// window name
  char const * windowNameCr = "Cr Channel";     // window name
  char const * windowNameCb = "Cb Channel"; 	// window name

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
      // create window object (use flag=0 to allow resize, 1 to auto fix size)

      cvNamedWindow(windowName, 0);
	  cvNamedWindow(windowNameY, 0);
	  cvNamedWindow(windowNameCr, 0);
	  cvNamedWindow(windowNameCb, 0);

	  // define required channel images for display processing
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

	  // setup channel images

	  YCrCbimg = cvCloneImage(img);

	  IplImage* y_channel = cvCreateImage(cvSize(img->width, img->height),
	  															img->depth, 1);
	  IplImage* cr_channel = cvCreateImage(cvSize(img->width, img->height),
	  															img->depth, 1);
	  IplImage* cb_channel = cvCreateImage(cvSize(img->width, img->height),
	  															img->depth, 1);
	  y_channel->origin = img->origin;
	  cr_channel->origin = img->origin;
	  cb_channel->origin = img->origin;

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

		  }	else {

			  // if not a capture object set event delay to zero so it waits
			  // indefinitely (as single image file, no need to loop)

			  EVENT_LOOP_DELAY = 0;
		  }

		  // colour transform images

		  cvCvtColor(img, YCrCbimg, CV_BGR2YCrCb);

		  // split image into channels remembering BGR ordering of OpenCV

		  cvSplit(YCrCbimg, y_channel, cr_channel, cb_channel, NULL);

		  // display images in windows

		  cvShowImage( windowName, img );
		  cvShowImage( windowNameY, y_channel );
		  cvShowImage( windowNameCr, cr_channel );
		  cvShowImage( windowNameCb, cb_channel );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 4 ms roughly equates to 100ms/25fps = 4ms per frame

		  // here we take account of processing time for the loop by subtracting the time
          // taken in ms. from this (1000ms/25fps = 40ms per frame) value whilst ensuring
          // we get a +ve wait time

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
	  cvReleaseImage( &y_channel );
	  cvReleaseImage( &cr_channel );
	  cvReleaseImage( &cb_channel );
	  cvReleaseImage( &YCrCbimg );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
