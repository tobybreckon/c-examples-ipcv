// Example : watershed segmentation
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>
#include <limits.h>

using namespace std;
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

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object
  CvCapture* capture = NULL; // capture object

  char* windowName = "Watershed Segmentation"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  int lowerThreshold = 50;  // lower canny edge threshold initial setting
  int upperThreshold = 200; // upper canny edge threshold initial setting
  int windowSize = 3;		// canny edge window size

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
  	  cvCreateTrackbar("Lower", windowName, &lowerThreshold, 255, NULL);
  	  cvCreateTrackbar("Upper", windowName, &upperThreshold, 255, NULL);
  	  cvCreateTrackbar("Window", windowName, &windowSize, 7, NULL);

	  // define working images

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
	  IplImage* edges =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  edges->origin = img->origin;
	  IplImage* output = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_32S, 1);
	  output->origin = img->origin;

	  IplImage* wshed = cvCloneImage( img );

	  // create a set of random colour labels

	  CvRNG rng = cvRNG(-1);
	  CvMat* color_tab = cvCreateMat( 1, 255, CV_8UC3 );
            for(int i = 0; i < 255; i++ )
            {
                uchar* ptr = color_tab->data.ptr + i*3;
                ptr[0] = (uchar)(cvRandInt(&rng)%180 + 50);
                ptr[1] = (uchar)(cvRandInt(&rng)%180 + 50);
                ptr[2] = (uchar)(cvRandInt(&rng)%180 + 50);
            }
	  CvMemStorage* storage = cvCreateMemStorage(0);

      CvSeq* contours = 0;
	  CvSeq* current_contour;
      int i, j, comp_count = 0;

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

		  // ***

			  // if input is not already grayscale, convert to grayscale

				  if (img->nChannels > 1){
					  cvCvtColor(img, grayImg, CV_BGR2GRAY);
				  } else {
					  grayImg = img;
				  }


			// check that the window size is always odd and > 1

			if ((fmod((double) windowSize, 2) == 0) || (windowSize <= 2)) {
				windowSize++;
			}

			// do canny edge detection then contour (edge tracking based
			// contour detection)

			cvCanny(grayImg, edges, lowerThreshold, upperThreshold, max(3, windowSize));

            cvFindContours(edges, storage, &contours, sizeof(CvContour),
                            CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
            cvZero( output );

			// draw the contours in the output image

			current_contour = contours;
			comp_count = 0;
            for( ; current_contour != 0; current_contour = current_contour->h_next, comp_count++ )
            {
                cvDrawContours( output, current_contour, cvScalarAll(comp_count+1),
                                cvScalarAll(comp_count+1), -1, -1, 8, cvPoint(0,0) );
            }

			if (contours != NULL){
				cvClearSeq(contours);
			}
			cvClearMemStorage(storage);

			// do the watershed segmentation

            cvWatershed( img, output );

            // paint the watershed image with colour labels

			cvZero(wshed);
            for( i = 0; i < output->height; i++ )
                for( j = 0; j < output->width; j++ )
                {
                    int idx = CV_IMAGE_ELEM( output, int, i, j );
                    uchar* dst = &CV_IMAGE_ELEM( wshed, uchar, i, j*3 );
                    if( idx == -1 ) {
                        dst[0] = dst[1] = dst[2] = (uchar)255;
                    } else if( idx <= 0 || idx > 255 ) {
                        dst[0] = dst[1] = dst[2] = (uchar)0; // should not get here
                    } else {
                        uchar* ptr = color_tab->data.ptr + (idx-1)*3;
                        dst[0] = ptr[0]; dst[1] = ptr[1]; dst[2] = ptr[2];
                    }
                }

		   	cvAddWeighted( wshed, 0.5, img, 0.5, 0, wshed );

		  // ***

		  // display image in window

		  cvShowImage( windowName, wshed );


		  // start event processing loop (very important,in fact essential for GUI)
	      // 4 ms roughly equates to 100ms/25fps = 4ms per frame

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
	  cvReleaseImage( &edges );
	  cvReleaseImage( &output );
	  cvReleaseImage( &wshed );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
