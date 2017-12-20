// Example : histogram equalise colour image
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
  CvCapture* capture = NULL; // capture object

  IplImage* eqHistogramImage = NULL;  	// histogram images
  IplImage* hsvHistogramImage = NULL;

  char const * windowName = "Histogram Equalization"; // window name
  char const * windowName1 = "Colour Image"; // window name
  char const * windowNameH1 = "Equalised Histogram"; // window name
  char const * windowNameH2 = "Original Histogram"; // window name

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
				printf("ERROR: cannot get next frame from camera\n");
			}
			exit(0);
		  }

	  }
	  if (img->nChannels != 3)
	  {
		printf("ERROR: input is not a colour image\n");
		exit(1);
	  }
	  IplImage* hsvImg =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 3);
	  IplImage* h_channel =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  IplImage* s_channel =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  IplImage* v_channel =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  hsvImg->origin = img->origin;

	  IplImage* eqImg =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 3);
	  eqImg->origin = img->origin;

	  eqHistogramImage = cvCreateImage(cvSize(255,200), 8, 1);
	  hsvHistogramImage = cvCreateImage(cvSize(255,200), 8, 1);

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

		  // *** Histogram Equalisation Processing

			  // convert colour RGB image to HSV

			  cvCvtColor(img, hsvImg, CV_BGR2HSV);

			  // seperate out the variance colour component

			  cvSplit(hsvImg, h_channel, s_channel, v_channel, NULL);

			  // create histogram prior to equalisation

			  create_histogram_image(v_channel, hsvHistogramImage);

			  // histogram equalize variance

			  cvEqualizeHist(v_channel, v_channel);

			  // create histogram after equalisation

			  create_histogram_image(v_channel, eqHistogramImage);

			  // copy it back to

			  cvMerge(h_channel, s_channel, v_channel, NULL, hsvImg);

			  cvCvtColor(hsvImg, eqImg, CV_HSV2BGR);

		  // display image in window

		  cvShowImage( windowName,  eqImg );
		  cvShowImage( windowName1, img );

		  cvShowImage( windowNameH1,  eqHistogramImage );
		  cvShowImage( windowNameH2, hsvHistogramImage );

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
	  cvReleaseImage( &hsvImg );
	  cvReleaseImage( &eqImg );

	  // release histogram images

	  cvReleaseImage( &eqHistogramImage );
      cvReleaseImage( &hsvHistogramImage );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
