// Example : basic moment based recognition from video / camera
//           using all three R, G, B colour channels
// usage: prog {<video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2010 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>    // standard C/C++ includes
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

void printhelp(){
	printf("\nControls: \n");
	printf("\tspace = capture a sample image\n");
	printf("\treturn = move to recognition mode (or m)\n");
	printf("\tr = recognise current image\n");
	printf("\tany key = clear recognition result\n");
	printf("\tx = exit\n");
}

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object
  CvCapture* capture = NULL; // capture object

  char const * windowName = "Moment Based Recognition"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame
  // max sample images

  #define MAX_NUMBER_OF_SAMPLE_IMAGES 255

  // input images

  IplImage* input[MAX_NUMBER_OF_SAMPLE_IMAGES];


  int imagesCollected = 0;			// number of sample images collected

  bool recognitionStage = false;	// flag to determine when have started
  									// recognition

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( CAMERA_INDEX )) != 0 )
	  )
    {
	  // print user controls

	  printhelp();

      // create window object (use flag=0 to allow resize, 1 to auto fix size)

      cvNamedWindow(windowName, 0);

	  // set up font structure

	  CvFont font;
	  cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1, 1 , 0, 2, 8 );

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

	  if (img->nChannels != 3)
	  {
		printf("ERROR: input is not a colour image\n");
		exit(1);
	  }

	  // create 2 sets of RGB colour channels for comparision

	  IplImage* channelR =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  IplImage* channelG =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  IplImage* channelB =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  IplImage* channelR_2 =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  IplImage* channelG_2 =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  IplImage* channelB_2 =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);

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

		  // display image in window (with text)

	      if (!recognitionStage){
          	cvPutText(img, "SAMPLE COLLECTION",
			  		cvPoint(10,img->height - 20), &font, CV_RGB(0, 255,0));
		  } else {
			cvPutText(img, "RECOGNITION",
			  		cvPoint(10,img->height - 20), &font, CV_RGB(255, 0 ,0));
		  }
		  cvShowImage( windowName, img );

		  // start event processing loop (very important,in fact essential for GUI)
	      // 40 ms roughly equates to 1000ms/25fps = 4ms per frame

		  // here we take account of processing time for the loop by subtracting the time
          // taken in ms. from this (1000ms/25fps = 40ms per frame) value whilst ensuring
          // we get a +ve wait time

          key = waitKey((int) std::max(2.0, EVENT_LOOP_DELAY -
                        (((getTickCount() - timeStart) / getTickFrequency()) * 1000)));

		  if (key == 'x'){

	   		// if user presses "x" then exit

	   			printf("Keyboard exit requested : exiting now - bye!\n");
	   			keepProcessing = false;

		  }  else if (key == ' '){

	   		// if user presses " " then capture a sample image

			if (!recognitionStage) {
				 if (imagesCollected < MAX_NUMBER_OF_SAMPLE_IMAGES)
				 {
					// copy image and store it

				    input[imagesCollected] = cvCloneImage(img);

					imagesCollected++;

					printf("Sample image collected - %i\n", imagesCollected);

				 } else {
					printf("ERROR: Maximum sample images (%d) collected.\n",
													 imagesCollected);
				 }
		   }

		  } else if ((key == '\n') || (key == 'm')) { // use "m" in windows

	   		// if user presses return then move into recognition mode

		    printf("Entering recognition mode - input images stored\n\n");

		    recognitionStage = true;
		    if (!(imagesCollected > 0)) {
			    printf("ERROR: not enough samples images caputured\n");
		    }
		  } else if (key == 'r'){

			// if user presses "r" then do recognition

			  // get the colour channels (rememeber - BGR!)

			  cvSplit(img, channelB, channelG, channelR, NULL);

			  // compare images using Hu moments of colour image channels

			  if (recognitionStage) {

				// for stored image do the comparison

				double closestDistance = HUGE;
				int closestImage = 0;

				for (int i = 0; i < imagesCollected; i++)
				{
					// get the colour channels (rememeber - BGR!)

					cvSplit(input[i], channelB_2, channelG_2, channelR_2, NULL);

					// see opencv manual for how cvMatchShapes() works but essentially
					// we are doing SUM(Mi1 - Mi2) for images for Hu moment i = {1 ... 7}
					// and images 1 to 7
					// Hu moments are position, orientation and scale invarient
					// moments of shape derived from the central normalised moments

					double diff = (cvMatchShapes(channelB, channelB_2,
                      								CV_CONTOURS_MATCH_I2, 0 ) +
								  cvMatchShapes(channelG, channelG_2,
                      								CV_CONTOURS_MATCH_I2, 0 ) +
								  cvMatchShapes(channelR, channelR_2,
                      								CV_CONTOURS_MATCH_I2, 0 ));

					printf("Comparison image %i = %.3f\n", i, diff);

					if (diff < closestDistance){
						closestDistance = diff;
						closestImage = i;

					}
				}

				printf("\n");

				// output the result in a window

				printf("Recognition - closest matching image = %d\n", closestImage);
				printf("Press any key to clear. \n\n");

				cvNamedWindow("Recognition Result", 1 );
                cvShowImage("Recognition Result", input[closestImage]);
				cvWaitKey(0);
				cvDestroyWindow("Recognition Result");

			} else {
				printf("ERROR - need to enter recognition stage first.\n");
			}
		  }
	  }

      // destroy window objects
      // (triggered by event loop *only* window is closed)

      cvDestroyAllWindows();

      // destroy image objects (if it does not originate from a capture object)

      if (!capture){
		  cvReleaseImage( &img );
	  } else {
		  cvReleaseCapture ( &capture );
	  }
	  cvReleaseImage(&channelB);
	  cvReleaseImage(&channelG);
	  cvReleaseImage(&channelR);
	  cvReleaseImage(&channelB_2);
	  cvReleaseImage(&channelG_2);
	  cvReleaseImage(&channelR_2);

	  for (int i = 0; i < imagesCollected; i++)
		  {
			  cvReleaseImage( &(input[i]));
		  }

	  // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
