// Example : grab and display a single live image
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

  char const * windowName = "OPENCV: live image display"; // window name

  // create window object

  cvNamedWindow(windowName, 1 );

  // grab an image from camera (here assume only 1 camera, device #0)

  CvCapture* capture = cvCaptureFromCAM(0); // capture from video device #0

  // Add in the following 2 lines for Quickcam Express II on linux

  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);


  if(!cvGrabFrame(capture)){
    printf("Could not grab a frame\n");
    exit(0);
  }

  // cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
  // cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 160);


  img=cvRetrieveFrame(capture);           // retrieve the captured frame

  // display image in window

  cvShowImage( windowName, img );

  // start event processing loop (very important,in fact essential for GUI)

  cvWaitKey(0);

  // destroy window object
  // (triggered by event loop *only* window is closed)

  cvDestroyWindow( windowName );

  // release capture device

  cvReleaseCapture(&capture);

  // Note that the image captured by the device is allocated/released
  // by the capture function.
  // There is no need to release it explicitly.

  // all OK : main returns 0

  return 0;

}
