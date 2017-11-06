// Example : example of colour / shape extraction
// usage: prog <video_name>

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : TBA

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>
#include <algorithm> // contains max() function (amongst others)
using namespace cv; // use c++ namespace so the timing stuff works consistently
using namespace std;

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

  char const * windowNameHSV = "Colour Information"; // window name
  char const * windowNameCanny = "Shape Information"; // window name

 IplImage* HSV = NULL;
 IplImage* singleChannelH = NULL;
 IplImage* singleChannelPlain = NULL;
 IplImage* cannyImg = NULL;
 IplImage* dst = NULL;
 int cannyLower = 100;
 int cannyUpper = 180;


  bool keepProcessing = true;	// loop control flag
  char key = '\0';				// user input
  int  EVENT_LOOP_DELAY = 40;    // 40 ms equates to 1000ms/25fps = 40ms per frame

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], 1)) != 0 ) ||
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( CAMERA_INDEX )) != 0 )
	  )
    {
      // create window object (use flag=0 to allow resize, 1 to auto fix size)

	  cvNamedWindow(windowNameHSV, 1);
	  cvNamedWindow(windowNameCanny, 1);

	  cvResizeWindow(windowNameHSV, 640, 480);
	  cvResizeWindow(windowNameCanny, 640, 480);
	  cvMoveWindow(windowNameHSV, 0, 100);
  	  cvMoveWindow(windowNameCanny, 500, 20);


	  //setup trackbars	- canny edge detection

	  cvCreateTrackbar("Lower", windowNameCanny, &cannyLower, 255, NULL);
	  cvCreateTrackbar("Upper", windowNameCanny, &cannyUpper, 255, NULL);

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

	  // setup output image - RGB assumed

	  HSV = cvCloneImage(img);
	  dst = cvCloneImage(img);
	  singleChannelH = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
	  singleChannelH->origin = img->origin;
	  singleChannelPlain = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
	  singleChannelPlain->origin = img->origin;
	  cannyImg = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
	  cannyImg->origin = img->origin;

	  cvSet(singleChannelPlain, cvScalar(255));

	  CvMemStorage* storage = cvCreateMemStorage(0);
      CvSeq* contours = 0;
	  CvSeq* current_contour;

	  // start main loop

	  while (keepProcessing) {

          int64 timeStart = getTickCount(); // get time at start of loop

		  // if capture object in use (i.e. video/camera)
		  // get image from capture object

		  if (capture) {

			  // cvQueryFrame is just a combination of cvGrabFrame
			  // and cvRetrieveFrame in one call.

			  img = cvQueryFrame(capture);

			  // cvQueryFrame s just a combination of cvGrabFrame
			  // and cvRetrieveFrame in one call.

			  if(!img){
				if (argc == 2){
					printf("End of video file reached\n");
				} else {
					printf("ERROR: cannot get next frame from camera\n");
				}
				exit(0);
			  }

		  }

		    // convert to HSV and extract the HUE as primary wavelength

				cvCvtColor(img, HSV, CV_BGR2HSV);
				cvSetImageCOI(HSV, 1); // channel 1, 0 means all channels
				cvCopy(HSV, singleChannelH);

		  		// do histogram equalisation (makes it look more impressive)

		        cvEqualizeHist(singleChannelH, singleChannelH);

                // put it all back together in RGB

				cvMerge(singleChannelH, singleChannelPlain, singleChannelPlain, NULL, HSV);
				cvCvtColor(HSV, HSV, CV_HSV2BGR);

				// do canny to get the shape (re-use single channel mem. space)

				cvCvtColor(img, singleChannelH, CV_BGR2GRAY);
				cvCanny(singleChannelH, cannyImg, cannyLower, cannyUpper, 3);


			    // find the contours

		  		cvFindContours( cannyImg, storage,
		  			&contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

		  		// draw the contours in the output image

		    	cvZero(dst);

            	for(current_contour = contours; current_contour != 0;
									current_contour = current_contour->h_next)
            	{
                	cvDrawContours( dst, current_contour, CV_RGB( 0, 255, 0 ),
                    			            CV_RGB( 0, 255, 0 ), -1, 1, 8, cvPoint(0,0) );
            	}
				if (contours != NULL){
					cvClearSeq(contours);
				}

			  // display image in window

			  cvShowImage( windowNameHSV, HSV );
			  cvShowImage( windowNameCanny, dst );


			  // start event processing loop (very important,in fact essential for GUI)

              // here we take account of processing time for the loop by subtracting the time
              // taken in ms. from this (1000ms/25fps = 40ms per frame) value whilst ensuring
              // we get a +ve wait time


              key = waitKey((int) std::max(2.0, EVENT_LOOP_DELAY -
                        (((getTickCount() - timeStart) / getTickFrequency()) * 1000)));;

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

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
