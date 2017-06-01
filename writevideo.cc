// Example : grab and write a video file
// usage: prog <output_video>

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2006 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>

int main( int argc, char** argv )
{

  IplImage* img;  // image object 

  // check that command line arguments are provided
	
    if( argc == 2 )
    {	

	  // here we will use a connected camera as the 
	  // originating source for our video file
	
	  CvCapture* capture = cvCaptureFromCAM(0); // capture from video device #0
	  if(!cvGrabFrame(capture)){              
		printf("Could not grab a frame\n");
		exit(0);
	  }
	  img=cvRetrieveFrame(capture);
	
	  // set up video writer object (using properties of camera capture source)
	  // N.B. we use "CV_FOURCC('D','I','V','X')" specify an MPEG-4 encoded video
	  
	  CvVideoWriter *writer = NULL;
	  int isColor = 1;
	  int fps     = 5;				// N.B. low frame rate
	  int frameW  = img->width;
	  int frameH  = img->height;
	  writer=cvCreateVideoWriter(argv[1],CV_FOURCC('D','I','V','X'),
								 fps,cvSize(frameW,frameH),isColor);
	  
	  // loop and store up to N frames
	
	  int nFrames = 50;
	
	  printf("\nStarting video capture........"); // signal start to user
	  fflush(NULL);
	  
	  for (int i=0;i<nFrames;i++){
		  
		// grab frame, retreive frame and write to video writer
		  
		cvGrabFrame(capture);
		img=cvRetrieveFrame(capture); 
		cvWriteFrame(writer,img);		
	  }
	  
	  printf("Stopped\n"); // signal end to user
	  
	  // release the video writer (closes file, de-allocates etc, etc.)
	  
	  cvReleaseVideoWriter(&writer);
	  
	  // release capture device
	
	  cvReleaseCapture(&capture);
	
	  // Note that the image captured by the device is allocated/released 
	  // by the capture function. 
	  // There is no need to release it explicitly.
		
	  // all OK : main returns 0
	  
	  return 0;
	}

  // not OK : main returns -1

  return -1;
}
