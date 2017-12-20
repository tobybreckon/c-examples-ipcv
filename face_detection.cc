// Example : face detection of image / video / camera
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include "cvaux.h"	  // open cv auxillary functions

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
  CvCapture* capture = NULL; // capture object

  char const * windowName = "Face Detection"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

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

	  // create grayscale image

	  IplImage* grayImg = cvCreateImage(cvSize(img->width,img->height),
				img->depth, 1);
	  grayImg->origin = img->origin;

	  // define face detection object(s)

	  CvSeq* faces = NULL;
	  CvMemStorage* storage = cvCreateMemStorage(0);

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

		  }

		  // if input is not already grayscale, convert to grayscale

		  if (img->nChannels > 1){
			 cvCvtColor(img, grayImg, CV_BGR2GRAY);
	      } else {
			grayImg = img;
		  }

		  cvEqualizeHist(grayImg, grayImg);

		  // use (undocumented) opencv face detection

		  faces = cvFindFace(grayImg, storage);

		  // faces = cvPostBoostingFindFace(grayImg, storage); is faster but unstable

		  // Note: perhaps post boosting should be post Haar based classifier?

		  CvFace face;

		  while ((faces) && (faces->total > 0))
		  {
			  cvSeqPop(faces, &face);

			  // mouth

			  cvRectangle(img, cvPoint(face.MouthRect.x, face.MouthRect.y),
			  cvPoint(face.MouthRect.x + face.MouthRect.width , face.MouthRect.y + face.MouthRect.height),
			  CV_RGB(0,0,255), 2, 1, 0);

			  //eyes

			  cvRectangle(img, cvPoint(face.LeftEyeRect.x, face.LeftEyeRect.y),
			  cvPoint(face.LeftEyeRect.x + face.LeftEyeRect.width , face.LeftEyeRect.y + face.LeftEyeRect.height),
			  CV_RGB(0,0,255), 2, 1, 0);
			  cvRectangle(img, cvPoint(face.RightEyeRect.x, face.RightEyeRect.y),
			  cvPoint(face.RightEyeRect.x + face.RightEyeRect.width , face.RightEyeRect.y + face.RightEyeRect.height),
			  CV_RGB(0,0,255), 2, 1, 0);

			  // face

			  cvRectangle(img, cvPoint(face.LeftEyeRect.x - 15, face.LeftEyeRect.y -15),
			  cvPoint(face.RightEyeRect.x + face.RightEyeRect.width + 25 , face.MouthRect.y + face.MouthRect.height + 25),
			  CV_RGB(255,0,255), 2, 1, 0);

	  	  }
          cvClearSeq(faces);
		  cvClearMemStorage(storage);

		  // display image in window

		  cvShowImage( windowName, img );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 40 ms roughly equates to 1000ms/25fps = 4ms per frame

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

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
