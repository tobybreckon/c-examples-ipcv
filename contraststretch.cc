// Example : contrast stretch grayscale image
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2009 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>
#include <algorithm> // contains max() function (amongst others)
using namespace cv; // use c++ namespace so the timing stuff works consistently

/******************************************************************************/
// setup the camera index properly based on OS platform

// 0 in linux gives first camera for v4l
//-1 in windows gives first device or user dialog selection

#ifdef linux
	#define CAMERA_INDEX 0
#else
	#define CAMERA_INDEX -1
#endif

/******************************************************************************/

// function that takes a gray scale image and draws a histogram
// image for it in a pre-allocated image

void create_histogram_image(IplImage* grayImg, IplImage* histogramImage){

  CvHistogram *hist = NULL;	    // pointer to histogram object
  float max_value = 0;			// max value in histogram
  int hist_size = 256;			// size of histogram (number of bins)
  int bin_w = 0;				// initial width to draw bars
  float range_0[]={0,256};
  float* ranges[] = { range_0 };

  hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);

  cvCalcHist( &grayImg, hist, 0, NULL );
  cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
  cvScale( hist->bins, hist->bins, ((double)histogramImage->height)/max_value, 0 );
  cvSet( histogramImage, cvScalarAll(255), 0 );
  bin_w = cvRound((double)histogramImage->width/hist_size);

  for(int i = 0; i < hist_size; i++ )
  {
     cvRectangle( histogramImage, cvPoint(i*bin_w, histogramImage->height),
                  cvPoint((i+1)*bin_w, histogramImage->height
	  								- cvRound(cvGetReal1D(hist->bins,i))),
                   					cvScalarAll(0), -1, 8, 0 );
  }

  cvReleaseHist (&hist);
}

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object

  IplImage* stHistogramImage = NULL;  	// histogram images
  IplImage* grayHistogramImage = NULL;

  CvCapture* capture = NULL; // capture object

  char const * windowName = "Contrast Stretched Version"; // window name
  char const * windowName1 = "Original Grayscale"; // window name
  char const * windowNameH1 = "Contrast Stretched Histogram"; // window name
  char const * windowNameH2 = "Original Grayscale Histogram"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( 0 )) != 0 )
	  )
    {
      // create window object (use flag=0 to allow resize, 1 to auto fix size)

      cvNamedWindow(windowName, 0);
	  cvNamedWindow(windowName1, 0);
	  cvNamedWindow(windowNameH1, 0);
	  cvNamedWindow(windowNameH2, 0);

	  // define required images for intermediate processing
      // (if using a capture object we need to get a frame first to get the size)

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

	  IplImage* grayImg =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  grayImg->origin = img->origin;
	  IplImage* stImg =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  stImg->origin = img->origin;

	  stHistogramImage = cvCreateImage(cvSize(255,200), 8, 1);
	  grayHistogramImage = cvCreateImage(cvSize(255,200), 8, 1);

	  // start main loop

	  while (keepProcessing) {

          int64 timeStart = getTickCount(); // get time at start of loop

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

		  }	else {

			  // if not a capture object set event delay to zero so it waits
			  // indefinitely (as single image file, no need to loop)

			  EVENT_LOOP_DELAY = 0;
		  }

		  // *** contrast stretch Processing

			  // if input is not already grayscale, convert to grayscale

			  if (img->nChannels > 1){
				  cvCvtColor(img, grayImg, CV_BGR2GRAY);
			  } else {
				  grayImg = img;
			  }

			  // contrast stretch it (cvNormalize() with these parameters does
			  // basic constrast stretching - see opencv manual)

			  cvNormalize(grayImg, stImg, 0, 255, CV_MINMAX);

		  // ***

		  // *** draw histograms

		  create_histogram_image(grayImg, grayHistogramImage);
		  create_histogram_image(stImg, stHistogramImage);

		  // display images in window

		  cvShowImage( windowName,  stImg );
		  cvShowImage( windowName1, grayImg );

		  cvShowImage( windowNameH1,  stHistogramImage );
		  cvShowImage( windowNameH2, grayHistogramImage );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 4 ms roughly equates to 100ms/25fps = 4ms per frame

		  // here we take account of processing time for the loop by subtracting the time
          // taken in ms. from this (1000ms/25fps = 40ms per frame) value whilst ensuring
          // we get a +ve wait time


          key = cvWaitKey((int) std::max(2.0, EVENT_LOOP_DELAY -
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
	  cvReleaseImage( &stImg );

	  // release histogram images

	  cvReleaseImage( &stHistogramImage );
      cvReleaseImage( &grayHistogramImage );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
