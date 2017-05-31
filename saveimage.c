// Example : saving an image
// usage: prog <image_name> <output_img>

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2006 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

int main( int argc, char** argv )
{

  IplImage* inputImg = NULL;  // input image object 
  IplImage* outputImg = NULL;  // output image object

  // check that command line arguments are provided and image reads in OK

    if( argc == 3 && (inputImg = cvLoadImage( argv[1], 1)) != 0 )
    {
      
      // Create the output image (same size & depth as input)
      
      outputImg = cvCreateImage(cvSize(inputImg->width,inputImg->height), 
				inputImg->depth, inputImg->nChannels);

      // invert image_name
	  
	  cvNot(inputImg, outputImg);
		
	  // write out image to file 	
			
	  cvSaveImage(argv[2], outputImg);
		
      // destroy image object

      cvReleaseImage( &inputImg );
      cvReleaseImage( &outputImg );
      
      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
