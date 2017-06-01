// Example : eigen image based recognition image / video / camera
// usage: prog {<video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include "cvaux.h"    // aux. OpenCV funcionality

#include <stdio.h>    // standard C/C++ includes
#include <algorithm> // contains max() function (amongst others)
using namespace cv; // use c++ namespace so the timing stuff works consistently
using namespace std;

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
	printf("\tb = build Eigen model from sample images\n");
	printf("\tr = recognise current image\n");
	printf("\tx = exit\n");
}

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object
  CvCapture* capture = NULL; // capture object

  char const * windowName = "Eigenspace Based Image Recognition"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  // eigen image specific stuff

  #define NUMBER_OF_EIGENVECTORS_IN_USE 10
  #define MAX_NUMBER_OF_SAMPLE_IMAGES 255

  // data structures and matrices for eigen based face recognition

  IplImage* input[MAX_NUMBER_OF_SAMPLE_IMAGES];
  CvMat* pcaInputs = NULL;
  CvMat* average = NULL;
  CvMat* eigenValues = NULL;
  CvMat* eigens = NULL;
  CvMat* coefficients = NULL;
  CvMat* recogniseCoeffs = NULL;
  CvMat* recognise = NULL;

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

	  IplImage* grayImg =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  grayImg->origin = img->origin;

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

	  	  // flip the image (so movement on screen matches movement in window)

		  cvFlip(img, NULL, 1);

		  // if input is not already grayscale, convert to grayscale

			  if (img->nChannels > 1){
				  cvCvtColor(img, grayImg, CV_BGR2GRAY);
			  } else {
				  grayImg = img;
			  }


		  // display image in window (with text and targets etc.)

	      if (!recognitionStage){
          	cvPutText(img, "SAMPLE COLLECTION",
			  		cvPoint(10,img->height - 20), &font, CV_RGB(0, 255,0));
		  } else {
			cvPutText(img, "RECOGNITION",
			  		cvPoint(10,img->height - 20), &font, CV_RGB(255, 0 ,0));
		  }
		  cvCircle(img, cvPoint(img->width / 2, img->height / 2),
			  		min(img->width, img->height) / 2, CV_RGB(255, 0, 0), 1, 8, 0);

		  cvCircle(img, cvPoint(img->width / 2
		  				+ (int) floor(1.3 * (min(img->width, img->height) / 8)), img->height / 2),
			  		10, CV_RGB(255, 0, 0), 1, 8, 0);
		  cvCircle(img, cvPoint(img->width / 2
		  				- (int) floor(1.3 * (min(img->width, img->height) / 8)), img->height / 2),
			  		10, CV_RGB(255, 0, 0), 1, 8, 0);

		  cvLine(img, cvPoint(0, img->height / 2),
		  				cvPoint(img->width, img->height / 2),
			  		CV_RGB(0,0,255), 1, 8 , 0);
		  cvLine(img, cvPoint(0, (img->height / 2)
		  				+ min(img->width, img->height) / 4),
		  			cvPoint(img->width, (img->height / 2)
		  				+ min(img->width, img->height) / 4 ),
			  		CV_RGB(0,0,255), 1, 8 , 0);
		  cvLine(img, cvPoint(0, (img->height / 2)
		  				- min(img->width, img->height) / 4),
		  			cvPoint(img->width, (img->height / 2)
						- min(img->width, img->height) / 4),
			  			CV_RGB(0,0,255), 1, 8 , 0);
		  cvLine(img, cvPoint(img->width / 2, 0),
		  				cvPoint(img->width / 2, img->height),
			 	 		CV_RGB(0,0,255), 1, 8 , 0);
		  cvShowImage( windowName, img );

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
		  }  else if (key == ' '){

	   		// if user presses " " then capture an image

			if (!recognitionStage) {
				 if (imagesCollected < MAX_NUMBER_OF_SAMPLE_IMAGES)
				 {
					input[imagesCollected] = cvCloneImage(grayImg);
					imagesCollected++;
					printf("Sample image collected - %i\n", imagesCollected);
				 } else {
					printf("ERROR: Maximum sample images (%d) collected.\n",
													 imagesCollected);
				 }
		   }

		  } else if (key == 'b'){

	   		// if user presses "b" then build model and move
			// into recognition mode

			if (imagesCollected > 2) {

			printf("\nBuilding Eigenimage model for %i images ... ", imagesCollected);
			fflush(NULL);

			// construct all required matrix structures and populate
			// inputs with data

			pcaInputs = cvCreateMat(imagesCollected, (input[0]->width * input[0]->height), CV_8UC1);
			average = cvCreateMat(1, (input[0]->width * input[0]->height), CV_32FC1);
			eigenValues = cvCreateMat(1, min(pcaInputs->rows, pcaInputs->cols), CV_32FC1);
			eigens = cvCreateMat(eigenValues->cols, (input[0]->width * input[0]->height), CV_32FC1);
			coefficients = cvCreateMat(imagesCollected, eigens->rows, CV_32FC1);

			// construct required structures for later recognition

			recogniseCoeffs = cvCreateMat(1, eigens->rows, CV_32FC1);
			recognise = cvCreateMat(1, input[0]->width * input[0]->height, CV_8UC1);

			for (int i = 0; i < imagesCollected; i++){
				for (int j = 0; j < (input[0]->width * input[0]->height); j++){
					CV_MAT_ELEM(*pcaInputs, uchar, i, j) = (input[i])->imageData[(j)];
				}
			}

			// compute eigen image representation

		   cvCalcPCA(pcaInputs, average, eigenValues, eigens, CV_PCA_DATA_AS_ROW);

		   // compute eigen. co-efficients for all sample images and store

		   cvProjectPCA(pcaInputs, average, eigens, coefficients);

		   printf("Done (%d eigenvectors in use)\n",  eigens->rows);


		   // move to recognition mode

		   recognitionStage = true;
		   } else {
			   printf("ERROR: not enough samples images caputured\n");
		   }
		  } else if (key == 'r'){

			// if user presses "r" then do recognition

			  if (recognitionStage) {

				// project image to eigen space

			  	for (int j = 0; j < (input[0]->width * input[0]->height); j++){
					CV_MAT_ELEM(*recognise, uchar, 0, j) = (grayImg)->imageData[(j)];
				}

				cvProjectPCA(recognise, average, eigens, recogniseCoeffs);

				// check which set of stored sample co-efficients it is
				// closest too and then display the corresponding image

				double closestCoeffDistance = HUGE;
				int closestImage = 0;

				for (int i = 0; i < imagesCollected; i++)
				{
					double diff = 0;
					for(int j = 0; j < recogniseCoeffs->cols; j++)
					{
						diff += fabs(CV_MAT_ELEM(*coefficients, float, i, j)
								- CV_MAT_ELEM(*recogniseCoeffs, float, 0, j));
					}
					if (diff < closestCoeffDistance){
						closestCoeffDistance = diff;
						closestImage = i;

					}
				}

				printf("Recognition - closest matching image = %d\n", closestImage);
				cvNamedWindow("Recognition Result", 1 );
                cvShowImage("Recognition Result", input[closestImage]);
				cvWaitKey(0);
				cvDestroyWindow("Recognition Result");

			} else {
				printf("ERROR - need a build eigen model before recognition.\n");
			}
		  }
	  }

      // destroy window objects
      // (triggered by event loop *only* window is closed)

      cvDestroyAllWindows();

      // destroy image object (if it does not originate from a capture object)

      if (!capture){
		  cvReleaseImage( &img );
      }
	  cvReleaseImage(&grayImg);
	  for (int i = 0; i < imagesCollected; i++){cvReleaseImage( &(input[i]));}

	  // release matrix objects

	  cvReleaseMat( &pcaInputs);
	  cvReleaseMat( &average );
	  cvReleaseMat( &eigenValues );
	  cvReleaseMat( &eigens );
	  cvReleaseMat( &coefficients );
	  cvReleaseMat( &recogniseCoeffs );
	  cvReleaseMat( &recognise );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
