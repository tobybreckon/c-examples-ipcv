// Example : smooth an image
// usage: prog <image_name>

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2006 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

int main( int argc, char** argv )
{

  IplImage* inputImg = NULL;  // input image object 
  IplImage* outputImg = NULL;  // output image object

  char const * windowName = "OPENCV: blurred image"; // window name

  // check that command line arguments are provided and image reads in OK

    if( argc == 2 && (inputImg = cvLoadImage( argv[1], 1)) != 0 )
    {
      
      // Create the output image (same size & depth as input)
      
      outputImg = cvCreateImage(cvSize(inputImg->width,inputImg->height), 
				inputImg->depth, inputImg->nChannels);

      // blur the input image using a 5 x 5 mask and store in output image

      cvSmooth(inputImg, outputImg, CV_BLUR, 5, 5, 0, 0); 

      // create window object

      cvNamedWindow(windowName, 1 );
      
      // display image in window

      cvShowImage( windowName, outputImg);
      
      // start event processing loop (very important,in fact essential for GUI)

      cvWaitKey(0); 
      
      // destroy window object
      // (triggered by event loop *only* window is closed)

      cvDestroyWindow( windowName );

      // destroy image object

      cvReleaseImage( &inputImg );
      cvReleaseImage( &outputImg );
      
      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
