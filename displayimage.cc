// Fix input images for homography program
// usage: prog [--hluav | --ugv] <image1.ext> <image1.ext>
// Output images as <image1_XxY.ext> <mage1_XxY.ext>

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2006 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

int main( int argc, char** argv )
{

  IplImage* img;  // image object

  char const * windowName = "OPENCV: basic image display"; // window name

  // check that command line arguments are provided and image reads in OK

    if( argc == 2 && (img = cvLoadImage( argv[1], CV_LOAD_IMAGE_UNCHANGED)) != 0 )
    {
      // create window object

      cvNamedWindow(windowName, 1 );

      // display image in window

      cvShowImage( windowName, img );

      // start event processing loop (very important,in fact essential for GUI)

      cvWaitKey(0);

      // destroy window object
      // (triggered by event loop *only* window is closed)

      cvDestroyWindow( windowName );

      // destroy image object

      cvReleaseImage( &img );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
