// Example : query colour elements in an image
// usage: prog <image_name>

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2006 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>

/******************************************************************************/

void colourQueryMouseCallBack(int event, int x, int y, int flags, void* img)
{

	// get number of channels in image (3 for RGB / 1 for grayscale)
	// (N.B. more efficient to access and store once rather than
	//	on every (!) event)

	int imageChannels = ((IplImage*) img)->nChannels;

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN :

		// left button prints colour information at click location to stdout

		printf("Colour information at image location (%i, %i) = ( ", x, y);

		for(int i = 0; i < imageChannels; i++){

		// here we use the CV_IMAGE_ELEM macro to access image elements
		// N.B. An image is Row * Column * NChannels array

			printf("%i ",
			CV_IMAGE_ELEM((IplImage*) img, uchar, y, (x * imageChannels) + i));
		}

		printf(")\n");

		;
		break;
	case CV_EVENT_RBUTTONDOWN :

		// right button sets colour information at click location to white

		printf("Colour information at image location (%i, %i) set to white\n"
				, x, y);

		for(int i = 0; i < imageChannels; i++){

		// use CV_IMAGE_ELEM to access image elements
		// set each channel to 255 (i.e. white)

			CV_IMAGE_ELEM((IplImage*) img, uchar, y,
											(x * imageChannels) + i) = 255;

		}

		;
		break;
	// defaults:

		// all other events are ignored (other buttons,

	//	;
	//	break;
	}
}

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img;  // image object
  char key;
  bool keepProcessing = true;

  char const * windowName = "OPENCV: colour query"; // window name

  // check that command line arguments are provided and image reads in OK

    if( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 )
    {
      // create window object

      cvNamedWindow(windowName, 1 );

	  // set function to be executed everytime the mouse is clicked/moved

      cvSetMouseCallback(windowName, (CvMouseCallback) colourQueryMouseCallBack,
						 img);


	  // print out some helpful information about the image

	  printf("Image : (width x height) = (%i x %i)\n", img->width, img->height);
	  printf("\t Colour channels : %i\n", img->nChannels);

      // loop so that events are processed and the image constantly redisplayed
      // (N.B. A more efficient method is to only redisplay when an event has
	  //  changed the image - this is left as an exercise)

	  while (keepProcessing){

		// display image in window

		 cvShowImage( windowName, img );

      	// start event processing loop (very important,in fact essential for GUI)

      	key=cvWaitKey(20);

		// get any keyboard input given by the user and process it

		if (key == 'x'){

			// if user presses "x" then exit

			printf("Keyboard exit requested : exiting now - bye!\n");
			keepProcessing = false;
		}

      }

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

/******************************************************************************/
