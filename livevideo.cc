// Example : grab and display live video
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

  char* windowName = "OPENCV: live video display"; // window name

  // create window object

  cvNamedWindow(windowName, 1 );

  // grab an image from camera (here assume only 1 camera, device #0)

  CvCapture* capture = cvCaptureFromCAM(0); // capture from video device #0
  if(!cvGrabFrame(capture)){              
    printf("Could not grab a frame\n");
    exit(0);
  }

  // loop and display up to N frames

  int nFrames = 50;

  for (int i=0;i<nFrames;i++){

    img=cvRetrieveFrame(capture); // retrieve the captured frame

    // display image in window

    cvShowImage( windowName, img );
 
    // start event processing loop (very important,in fact essential for GUI)
    // Note that without the 40[msec] delay the captured sequence 
    // is not displayed properly.

    cvWaitKey(40); 

    // capture next frame

    cvGrabFrame(capture);

  }
  
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
