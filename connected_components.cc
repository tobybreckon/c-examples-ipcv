// Example : display connected components of an image / video / camera
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
// setup the camera index properly based on OS platform

// 0 in linux gives first camera for v4l
//-1 in windows gives first device or user dialog selection

#ifdef linux
	#define CAMERA_INDEX 1
#else
	#define CAMERA_INDEX -1
#endif

/******************************************************************************/

// sort connected components by bounding rectange size

/* static int sort_contour( const void* _a, const void* _b, void* userdata )
{

	printf("calling\n");

	if (
	    (cvArcLength((CvContour*) _a) > 0)
		&&
	    (cvArcLength((CvContour*) _b) > 0) &&

	    (
	     fabs(cvContourArea((CvContour*) _a)) >
             fabs(cvContourArea((CvContour*) _b))
	     )
	    )
	{
		printf("greater\n");
		return 1;
	}

	printf("not greater\n");

	return -1;
	} */

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img;  			 		// input image object
  IplImage* grayImg = NULL;  		// tmp image object
  IplImage* thresholdedImg = NULL;  // threshold output image object
  IplImage* dst;					// output connected components

  int windowSize = 3; // starting threshold value
  int constant = 0; // starting constant value
  CvCapture* capture = NULL; // capture object

  char const * windowName1 = "OPENCV: adaptive image thresholding"; // window name
  char const * windowName2 = "OPENCV: grayscale image"; // window name
  char const * windowName3 = "OPENCV: adaptive threshold image"; // window name


  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame



  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( CAMERA_INDEX )) != 0 )
	  )
    {
      // create window objects

      cvNamedWindow(windowName1, 0 );
      cvNamedWindow(windowName2, 0 );
      cvNamedWindow(windowName3, 0 );

	  // add adjustable trackbar for threshold parameter

      cvCreateTrackbar("Neighbourhood (N)", windowName3, &windowSize, 255, NULL);
	  cvCreateTrackbar("Constant (C)", windowName3, &constant, 50, NULL);

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

	  // create output image

	  thresholdedImg = cvCreateImage(cvSize(img->width,img->height),
						   img->depth, 1);
	  thresholdedImg->origin = img->origin;
	  grayImg = cvCreateImage(cvSize(img->width,img->height),
				img->depth, 1);
	  grayImg->origin = img->origin;

	  dst = cvCloneImage(img);

	  // create a set of random labels

	  CvRNG rng = cvRNG(-1);
	  CvMat* color_tab = cvCreateMat( 1, 255, CV_8UC3 );
            for(int i = 0; i < 255; i++ )
            {
                uchar * ptr = color_tab->data.ptr + i*3;
                ptr[0] = (uchar)(cvRandInt(&rng)%180 + 50);
                ptr[1] = (uchar)(cvRandInt(&rng)%180 + 50);
                ptr[2] = (uchar)(cvRandInt(&rng)%180 + 50);
            }
	  CvMemStorage* storage = cvCreateMemStorage(0);

      CvSeq* contours = 0;
	  CvSeq* current_contour;
      int comp_count = 0;

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

		  }

		  // if input is not already grayscale, convert to grayscale

		  if (img->nChannels > 1){
			 cvCvtColor(img, grayImg, CV_BGR2GRAY);
	      } else {
			grayImg = img;
		  }

		  // display image in window

		  cvShowImage( windowName2, grayImg );

		  // check that the window size is always odd and > 3

		  if ((windowSize > 3) && (fmod((double) windowSize, 2) == 0)) {
				windowSize++;
		  } else if (windowSize < 3) {
			  windowSize = 3;
		  }

		  // threshold the image and display

		  cvAdaptiveThreshold(grayImg, thresholdedImg, 255,
		  						CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,
		  						windowSize, constant);
		  cvShowImage( windowName3, thresholdedImg );

		 // find the contours

		  cvFindContours( thresholdedImg, storage,
		  		&contours, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

		  // if (contours) (cvSeqSort(contours, sort_contour, NULL));

		  // draw the contours in the output image

		    cvZero(dst);
			current_contour = contours;
			comp_count = 0;
            for( ; current_contour != 0; current_contour = current_contour->h_next, comp_count++ )
            {
				uchar const * ptr = color_tab->data.ptr + (comp_count)*3;
				CvScalar color = CV_RGB( ptr[0], ptr[1], ptr[2] );
                cvDrawContours( dst, current_contour, color,
                                color, -1, CV_FILLED, 8, cvPoint(0,0) );
            }
			if (contours != NULL){
				cvClearSeq(contours);
			}


		  // display images in window

		  cvShowImage( windowName1, dst );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 40 ms roughly equates to 1000ms/25fps = 40ms per frame

		  // here we take account of processing time for the loop by subtracting the time
          // taken in ms. from this (1000ms/25fps = 40ms per frame) value whilst ensuring
          // we get a +ve wait time


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

	  // destroy image objects

      cvReleaseImage( &grayImg );
	  cvReleaseImage( &thresholdedImg );
	  cvReleaseImage( &dst );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
