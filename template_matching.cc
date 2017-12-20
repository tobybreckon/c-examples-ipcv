// Example : template matching for video / camera
// usage: prog {<video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2009 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>
using namespace std;
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

// global variables (bad pratice yes, but an easy option for a simple demo)

static CvRect selection;		// the area selected by the mouse
static int select_object = 0;	// has an area been selected
static CvPoint origin;			// the origin of the selection
static int image_height;		// height of image
static int image_origin;		// image origin

/******************************************************************************/

// Mouse callback function: records a selected ROI in the image
// parameters:
// event - opencv mouse event type
// x - x position of mouse event
// y - y position of mouse event
// param - unused

// Notes: based on OpenCV 1.0 camshiftdemo.c example

void set_selected_roi( int event, int x, int y, int flags, void* param )
{
	// if we have already clicked and are selecting a region update
	// the global variable selection

	// first flip the y co-ordinate so we get the correct point
	// in the image (dependent on image origin)

	if (image_origin)
		y = image_height - y;

    if( select_object )
    {
        selection.x = min(x,origin.x);
        selection.y = min(y,origin.y);
        selection.width = selection.x + CV_IABS(x - origin.x);
        selection.height = selection.y + CV_IABS(y - origin.y);

        selection.x = max( selection.x, 0 );
        selection.y = max( selection.y, 0 );
        selection.width -= selection.x;
        selection.height -= selection.y;
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin = cvPoint(x,y);
        selection = cvRect(x,y,0,0);
        select_object = 1;
        break;
    case CV_EVENT_LBUTTONUP:
        select_object = 0;
        break;
    }
}

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object (main)
  IplImage* img_copy = NULL;      // image object (copy)
  IplImage* templateImg = NULL;      // image object (template)
  IplImage* response = NULL;      // image object (template)

  CvCapture* capture = NULL; // capture object

  char const * windowName = "Correlation Based Template Matching"; // window name
  char const * windowName2 = "Correlation Response"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  double min_val; double max_val;	// min/max response values for correlation
  CvPoint min_loc; CvPoint max_loc; // min/max response locations for correlation

  CvPoint detectionU; CvPoint detectionL; // Upper and Lower co-ordinates of template match

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (capture = cvCreateFileCapture( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( CAMERA_INDEX )) != 0 )
	  )
    {
      // create window object (use flag=0 to allow resize, 1 to auto fix size)

      cvNamedWindow(windowName, 0);
	  cvNamedWindow(windowName2, 0);

	  printf("\nPress 'c' to reset template\n\n");

	  // setup mouse callback

	  cvSetMouseCallback(windowName, set_selected_roi, 0 );

	  // set initial selection to zero

	  selection.width = 0;
	  selection.height = 0;

	  // start main loop

	  while (keepProcessing) {

          int64 timeStart = getTickCount(); // get time at start of loop

		  // capture object in use (i.e. video/camera)
		  // get image from capture object

			  img = cvQueryFrame(capture);
			  if(!img){
				if (argc == 2){
					printf("End of video file reached\n");
				} else {
					printf("ERROR: cannot get next fram from camera\n");
				}
				exit(0);
			  }
			  image_origin = img->origin;
			  image_height = img->height;

			  if (img_copy){
				cvReleaseImage( &img_copy);
			  }
			  img_copy = cvCloneImage(img);

		  // *** Template Matching Code

		    // if we have a selection then mark it on the image as
		    // we make it with the mouse

        	if( select_object && (selection.width > 0) && (selection.height > 0) )
       		{
                // first checking it is valid (>0) and within the image bounds

				selection.width = min( selection.width, img->width );
        		selection.height = min( selection.height, img->height );

				// set it as a Region of Interest (ROI) in the image

            	cvSetImageROI( img, selection );

				// use XOR with a scalar (255 for all channels to mark it out)

            	cvXorS( img, cvScalarAll(255), img, 0 );

				// reset ROI for image

            	cvResetImageROI( img );
        	}

			// if we have selected a new template within the image make a copy of it

			if ( (!(select_object)) && (selection.width > 0) && (selection.height > 0) &&
				 (!templateImg))
			{

				// make a copy of the ROI as the template (setting ROI as before) from the copy (not marked)

				templateImg = cvCreateImage(cvSize(selection.width, selection.height), img->depth, img->nChannels);
				templateImg->origin = img->origin;
				cvSetImageROI( img_copy, selection );
				cvCopy(img_copy, templateImg, NULL);
				cvResetImageROI( img_copy );

				// we can also now create the output space for the cross correlation
				// (N.B. the difference in dimensions indicates how the image borders will
				// be handled!)

				if (response){
					cvReleaseImage( &response);
				}
				response = cvCreateImage(cvSize(img->width - selection.width + 1, img->height - selection.height + 1),
									   IPL_DEPTH_32F, 1);
				response->origin = img->origin;

			}

			// if we have a selected template perform cross correlation template matching

			if (templateImg){

				// use match template function with normalised cross correlation specified

				cvMatchTemplate(img, templateImg, response, CV_TM_CCORR_NORMED);

				// find the maximal response

				cvMinMaxLoc(response, &min_val, &max_val, &min_loc, &max_loc, NULL);

				// draw the detected result

				detectionU.x = max_loc.x;
				detectionU.y = max_loc.y;
				detectionL.x = detectionU.x + templateImg->width;
				detectionL.y = detectionU.y + templateImg->height;
				cvRectangle(img, detectionU, detectionL, CV_RGB(255, 0, 0), 1, 8, 0);

			}

		  // ***

		  // display image in window

		  cvShowImage( windowName, img );
		  cvShowImage( windowName2, response );

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
		  } else if (key == 'c'){

	   		// if user presses "c" then clear template

				printf("Template reset by user request\n");

	   			if (templateImg)
				{
					cvReleaseImage( &templateImg );
					templateImg = NULL;
				}

				selection.width = 0;
				selection.height = 0;
		  }
	  }

      // destroy window objects
      // (triggered by event loop *only* window is closed)

      cvDestroyAllWindows();

      // destroy image objects (if it does not originate from a capture object)

      if (!templateImg){
		  cvReleaseImage( &templateImg );
      }

	  cvReleaseImage ( &img_copy);

	  if (!response){
		cvReleaseImage ( & response );
	  }

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
