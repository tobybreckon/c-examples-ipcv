// Example : histogram from image / video / camera
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>
#include <algorithm> // contains max() function (amongst others)
using namespace cv; // use c++ namespace so the timing stuff works consistently

/******************************************************************************/
// setup the cameras properly based on OS platform

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
  IplImage* grayImg = NULL;  // image object
  IplImage* hist_img = NULL; // histogram image object
  CvCapture* capture = NULL; // capture object

  char const * windowName = "Grayscale Image"; // window name
  char const * windowNameHist = "Grayscale Image Histogram"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  CvHistogram *hist = NULL;	    // pointer to histogram object
  float max_value = 0;			// max value in histogram
  int hist_size = 256;			// size of histogram (number of bins)
  int bin_w = 0;				// initial width to draw bars
  float range_0[]={0,256};
  float* ranges[] = { range_0 };

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], CV_LOAD_IMAGE_UNCHANGED)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( CAMERA_INDEX )) != 0 )
	  )
    {
      // create window object (use flag=0 to allow resize, 1 to auto fix size)

      cvNamedWindow(windowName, 0);
	  cvNamedWindow(windowNameHist, 0);

	  // create histogram + image to draw it in

	  hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
	  hist_img = cvCreateImage(cvSize(255,200), 8, 1);

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

	  // create grayscale image object

	  grayImg = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);

	  // start main loop

	  while (keepProcessing) {

          int64 timeStart = getTickCount(); // get time at start of loop

		  // if capture object in use (i.e. video/camera)
		  // get image from capture object

		  if (capture) {

			  // cvQueryFrame is just a combination of cvGrabFrame
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

		  // get a grayscale version of the image

		  if (img->nChannels > 1){
		  	cvCvtColor(img, grayImg, CV_BGR2GRAY);
		  } else {
			grayImg = img;
		  }
		  grayImg->origin = img->origin;

		  // calc and display histogram (from opencv demhist.c example)

		  cvCalcHist( &grayImg, hist, 0, NULL );
    	  cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
    	  cvScale( hist->bins, hist->bins, ((double)hist_img->height)/max_value, 0 );
    	  cvSet( hist_img, cvScalarAll(255), 0 );
    	  bin_w = cvRound((double)hist_img->width/hist_size);

    	  for(int i = 0; i < hist_size; i++ )
		  {
        		cvRectangle( hist_img, cvPoint(i*bin_w, hist_img->height),
            		         cvPoint((i+1)*bin_w, hist_img->height - cvRound(cvGetReal1D(hist->bins,i))),
                		     cvScalarAll(0), -1, 8, 0 );
		  }

		  // display image in window

		  cvShowImage(windowNameHist, hist_img );
		  cvShowImage( windowName, grayImg );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 40 ms roughly equates to 1000ms/25fps = 4ms per frame

		  // here we take account of processing time for the loop by subtracting the time
          // taken in ms. from this (1000ms/25fps = 40ms per frame) value whilst ensuring
          // we get a +ve wieght time

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
	  cvReleaseImage( &grayImg );
	  cvReleaseImage( &hist_img );
	  cvReleaseHist( &hist );


      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
