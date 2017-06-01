// Example : adaptive threshold an image / video / camera
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>

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
  IplImage* grayImg = NULL;  		// tmp image object
  IplImage* thresholdedImg = NULL;  // output image object
	
  int windowSize = 3; // starting threshold value
  int constant = 0; // starting constant value
  CvCapture* capture = NULL; // capture object
	
  char* windowName1 = "OPENCV: adaptive image thresholding"; // window name
  char* windowName2 = "OPENCV: grayscale image"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input	
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame	
	
  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera 

    if( 
	  ( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) || 
	  ( argc != 2 && (capture = cvCreateCameraCapture( CAMERA_INDEX )) != 
0 
)
	  )
    {
      // create window objects

      cvNamedWindow(windowName1, 1 );
      cvNamedWindow(windowName2, 1 );
		
	  // add adjustable trackbar for threshold parameter
		
      cvCreateTrackbar("Neighbourhood (N)", windowName1, &windowSize, 255, NULL);		
	  cvCreateTrackbar("Constant (C)", windowName1, &constant, 50, NULL);
		
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
	  
	  // create output image
	  
	  thresholdedImg = cvCreateImage(cvSize(img->width,img->height), 
						   img->depth, 1);
	  thresholdedImg->origin = img->origin;
	  grayImg = cvCreateImage(cvSize(img->width,img->height), 
				img->depth, 1);
	  grayImg->origin = img->origin;

	  // start main loop	
		
	  while (keepProcessing) {
		
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
	  
		  // display image in window

		  cvShowImage( windowName2, grayImg );
	
		  // check that the window size is always odd and > 3

		  if ((windowSize > 3) && (fmod((double) windowSize, 2) == 0)) {
				windowSize++;
		  } else if (windowSize < 3) {
			  windowSize = 3;
		  }
		  
		  // threshold the image
		  
		  cvAdaptiveThreshold(grayImg, thresholdedImg, 255,
		  						CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 
		  						windowSize, constant);
			
		  // display image in window

		  cvShowImage( windowName1, thresholdedImg );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 40 ms roughly equates to 1000ms/25fps = 40ms per frame
		  
		  key = cvWaitKey(EVENT_LOOP_DELAY);

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

      cvReleaseImage( &grayImg );
	  cvReleaseImage( &thresholdedImg );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
