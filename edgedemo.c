// Example : grab and display live video as an edge map
// usage: prog

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2006 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>

int main( int argc, char** argv )
{

  IplImage* img;  // image object 
  IplImage* grayImg = NULL;  // tmp image object
  IplImage* edgeImg = NULL;  // output image object

  char* windowName = "OPENCV: live edges"; // window name

  bool keepProcessing = true;
  char key;

  // create window object

  cvNamedWindow(windowName, 1 );

  // grab an image from camera (here assume only 1 camera, device #0)

  CvCapture* capture = cvCaptureFromCAM(0); // capture from video device #0
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

  while(keepProcessing)
  {
   
   img=cvRetrieveFrame(capture); // retrieve the captured frame

   cvCvtColor(img, grayImg, CV_BGR2GRAY);

   cvCanny(grayImg, edgeImg, 10, 200, 3);

   // display image in window

   cvShowImage(windowName, edgeImg);

   // start event processing loop

   // here we use a 4 msec delay for ~25 fps (100/25 = 4) 

    key = cvWaitKey(4);

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
