// Example : rotate and/or zoom an image / video / camera
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
  IplImage* zoomed = NULL;      // image object
  IplImage* imgOutput = NULL;      // image object
  CvCapture* capture = NULL; // capture object

  char const * windowName = "Rotate/Zoom Image"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  int angle = 0;
  int zoom = 1;

  int interpolation_type = CV_INTER_LINEAR;

  CvRect zoom_area;				// zoom region to zoom in on within the image

  CvMat* rot_matrix = cvCreateMat(2, 3, CV_32FC1 ); // rotation matrix
  CvPoint2D32f center;								// centre point for rotation

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
	  cvCreateTrackbar("angle", windowName, &angle, 360, NULL);
	  cvCreateTrackbar("zoom", windowName, &zoom, 10, NULL);

	  // capture an initial image to get dimensions

	  img = cvQueryFrame(capture);
	  if(!img){
		if (argc == 2){
				printf("End of video file reached\n");
		} else {
			printf("ERROR: cannot get next fram from camera\n");
		}
			exit(0);
	 }

	  zoomed = cvCloneImage(img);
	  imgOutput = cvCloneImage(img);

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

		  // *** ZOOM the image if zoom > 1

		  if (zoom > 1){

            // calculate a zoom sub-region (in the centre of the image)

			zoom_area.x = cvFloor((((img->width / zoom) * (zoom / 2.0)) - ((img->width / zoom) / 2.0)));
			zoom_area.y = cvFloor((((img->height / zoom) * (zoom / 2.0))- ((img->height / zoom) / 2.0)));

			zoom_area.width = cvFloor((img->width / zoom));
			zoom_area.height = cvFloor((img->height / zoom));

			// use ROI settings to zoom into it

			cvSetImageROI(img, zoom_area);
			cvResize(img, zoomed, interpolation_type);
			cvResetImageROI(img);

		  } else {

            // if no ZOOM then output = input

			cvCopy(img, zoomed, NULL);
		  }

		  // *** ROTATE image (N.B. angle is specified in degrees)

		  if (angle > 0)
		  {
			// get the centre and the rotation matrix

			center.x = (float) imgOutput->width/2;
			center.y = (float) imgOutput->height/2;
			cv2DRotationMatrix(center, (float) angle, 1.0, rot_matrix);

			// apply the rotation as an affine transform using cubic interpolation

			cvZero(imgOutput);
			cvWarpAffine(zoomed, imgOutput,
				rot_matrix, CV_INTER_CUBIC+CV_WARP_FILL_OUTLIERS, cvScalarAll(0));

		  } else {

			// if no ROTATE then output = input

			cvCopy(zoomed, imgOutput, NULL);
		  }

		  // ***

		  // display image in window

		  cvShowImage( windowName, imgOutput );

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
		  } else if (key == 'l'){
			  interpolation_type = CV_INTER_LINEAR;
			  printf("l - linear interpolation selected\n\n");
		  } else if (key == 'c'){
			  interpolation_type = CV_INTER_CUBIC;
			  printf("c - cubic interpolation selected\n\n");
		  } else if (key == 'n'){
			  interpolation_type = CV_INTER_NN;
			  printf("n - nearest neighbour interpolation selected\n\n");
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
