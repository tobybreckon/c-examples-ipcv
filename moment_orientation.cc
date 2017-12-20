// Example : get orientation from moments of largest closed contour
// in an image / video / camera
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2010 School of Engineering, Cranfield University
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

#define PI 3.14159265

/******************************************************************************/

// find the largest contour (by area) from a sequence of contours and return a
// pointer to that item in the sequence

CvSeq* findLargestContour(CvSeq* contours){

  CvSeq* current_contour = contours;
  double largestArea = 0;
  CvSeq* largest_contour = NULL;

  // check we at least have some contours

  if (contours == NULL){return NULL;}

  // for each contour compare it to current largest area on
  // record and remember the contour with the largest area
  // (note the use of fabs() for the cvContourArea() function)

  while (current_contour != NULL){

	  double area = fabs(cvContourArea(current_contour));

	  if(area > largestArea){
		  largestArea = area;
		  largest_contour = current_contour;
	  }

	  current_contour = current_contour->h_next;
  }

  // return pointer to largest

  return largest_contour;

}

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img;  			 		// input image object
  IplImage* grayImg = NULL;  		// tmp image object
  IplImage* thresholdedImg = NULL;  // thresholded image object
  IplImage* closeImage  = NULL;     // morphed image object
  IplImage* output = NULL; 		    // output image object

  int windowSize = 3; // starting threshold value
  int constant = 0; // starting constant value
  CvCapture* capture = NULL; // capture object

  char const * windowName1 = "OPENCV: adaptive image thresholding"; // window name
  char const * windowName2 = "OPENCV: Main Contour Image"; // window name
  char outputString[255]; // string for text output in a window

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  // initialise some contour finding stuff

  CvMemStorage* storage = cvCreateMemStorage(0);
  CvSeq* contours = 0;
  CvSeq* largest_contour = NULL;

  IplConvKernel* structuringElement =
  	cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);

  int iterations = 1;					 // closing iterations to apply

  CvMoments moments;					 // moments of shape

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( 0 )) != 0 )
	  )
    {
      // create window objects

      cvNamedWindow(windowName1, 0 );
      cvNamedWindow(windowName2, 0 );

	  // add adjustable trackbar for threshold parameter

      cvCreateTrackbar("Neighbourhood (N)", windowName1, &windowSize, 255, NULL);
	  cvCreateTrackbar("Constant (C)", windowName1, &constant, 50, NULL);
	  cvCreateTrackbar("Closing", windowName1, &iterations, 25, NULL);

	  // set up font structure

	  CvFont font;
	  cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1, 1 , 0, 2, 8 );

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

	  // create output images

	  thresholdedImg = cvCreateImage(cvSize(img->width,img->height),
						   img->depth, 1);
	  thresholdedImg->origin = img->origin;
	  grayImg = cvCreateImage(cvSize(img->width,img->height),
				img->depth, 1);
	  grayImg->origin = img->origin;

	  output = cvCloneImage(img);
	  closeImage = cvCloneImage(grayImg);

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

		  // check that the window size is always odd and > 3

		  if ((windowSize > 3) && (fmod((double) windowSize, 2) == 0)) {
				windowSize++;
		  } else if (windowSize < 3) {
			  windowSize = 3;
		  }

		  // threshold the image

		  cvAdaptiveThreshold(grayImg, thresholdedImg, 255,
		  						CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,
		  						windowSize, constant);


		  // morphological closing

		   if (iterations < 1) {iterations++;}

		   cvNot(thresholdedImg, thresholdedImg);
		   cvMorphologyEx(thresholdedImg, closeImage, NULL, structuringElement,
		  											CV_MOP_CLOSE, iterations);


		  // display image in window

		  cvShowImage( windowName1, closeImage );

		  // find the contours

		  cvFindContours( closeImage, storage,
		  		&contours, sizeof(CvContour), CV_RETR_EXTERNAL,
		   		CV_CHAIN_APPROX_SIMPLE );

		  //draw first main contour

		  cvCopy(img, output);

		  largest_contour = findLargestContour(contours);

          cvDrawContours( output, largest_contour,
		  			CV_RGB( 255, 0, 0 ), CV_RGB( 255, 0, 0 ),
		      0, 1, 8, cvPoint(0,0) );


		  // compute moment orientation

		  if (largest_contour != NULL){

			// first get regular variant moments

			cvMoments(largest_contour, &moments, 0 );

			// then get and display orientation from the central moments
			// use the CENTRAL moments (!), not the spatial moments

			// result appears to be the angle to the nearest axis (x or y)
			// (this is possible due to order of the contour vertices)

			double theta = 0.5 * atan(
			    (2 * cvGetCentralMoment(&moments, 1, 1)) /
			    (cvGetCentralMoment(&moments, 2, 0) -  cvGetCentralMoment(&moments, 0, 2)));
			theta = (theta / PI) * 180;

			// fit an ellipse (and draw it)

			if (largest_contour->total >= 6) // can only do an ellipse fit
				  							 // if we have > 6 points
			{
				CvBox2D box = cvFitEllipse2(largest_contour);
				if ((box.size.width < output->width) &&  (box.size.height < output->height))
				{
					// get the angle of the ellipse correct (taking into account MS Windows
					// seems to draw ellipses differently

                    // next line removed to make it draw correctly from OpenCV 2.3 onwards
					// box.angle = 270 - box.angle;
					#ifndef WIN32
						if( output->origin ) {
							box.angle = - box.angle;
						}
					#endif
					cvEllipseBox(output, box, CV_RGB(0, 255 ,0), 1, 8, 0 );
				}
			}

			#ifdef WIN32
				int height_offset = 20;
			#else
				int height_offset = 5;
			#endif
			sprintf(outputString, "angle: %.10f", theta);
				cvPutText(output, outputString,
			  		cvPoint(10,output->height - height_offset), &font, CV_RGB(0, 255,0));

		  }

		  // clear detected contours

		  if (contours != NULL){
				cvClearSeq(contours);
		  }

		  // display image in window

		  cvShowImage( windowName2, output );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 40 ms roughly equates to 1000ms/25fps = 40ms per frame

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

	  // destroy image objects

      cvReleaseImage( &grayImg );
	  cvReleaseImage( &thresholdedImg );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
