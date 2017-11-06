// Example : CAMSIFT based object tracking for video / camera
// usage: prog {<video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2009 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

// Notes: (heavily) based on OpenCV 1.0 camshiftdemo.c example

// Implements (using the OpenCV implementation):
// G.R. Bradski. Computer vision face tracking as a component of a
// perceptual user interface. In Workshop on Applications of Computer Vision,
// pages 214-219, Princeton, NJ, Oct. 1998.

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>
#include <algorithm>
using namespace std;
using namespace cv;

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

// hue to RGB conversion : coverts a given hue value to a RGB triplet for
// display
// parameters:
// hue - hue value in range 0 to 180 (OpenCV implementation of HSV)
// return value - CvScalar as RGB triple

// Notes: taken from OpenCV 1.0 camshiftdemo.c example


static CvScalar hue2rgb( float hue )
{
    int rgb[3], p, sector;
    static const int sector_data[][3]=
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object (main)
  IplImage* img_copy = NULL;      // image object (copy)

  CvCapture* capture = NULL; // capture object

  char const * windowName = "CAMSHIFT Tracking"; // window name
  char const * windowName2 = "Histogram Model (Hue)"; // window name
  char const * windowName3 = "Variance / Saturation Mask"; // window name
  char const * windowName4 = "Histogram Live (Hue)"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  CvRect track_window;			// tracking window used by CAMSHIFT
  CvBox2D track_box;			// tracking box used by CAMSHIFT
  CvConnectedComp track_comp;   // tracking component used by CAMSHIFT

  int vmin = 10, vmax = 256, smin = 30; // min/max values for Variance/Saturation mask


  bool histogramCaptured = false; // have we a histogram model to track ?
  bool showBackProjection = false; // have we a histogram model to track ?

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
	  cvNamedWindow(windowName3, 0);
	  cvNamedWindow(windowName4, 0);

	  cvCreateTrackbar( "Vmin", windowName3, &vmin, 256, 0 );
      cvCreateTrackbar( "Vmax", windowName3, &vmax, 256, 0 );
      cvCreateTrackbar( "Smin", windowName3, &smin, 256, 0 );

	  printf("\nPress 'c' to reset histogram model\n\n");
	  printf("\nPress 'b' to show backprojection\n\n");

	  // setup mouse callback

	  cvSetMouseCallback(windowName, set_selected_roi, 0 );

	  // set initial selection to zero

	  selection.width = 0;
	  selection.height = 0;

	 // do initial capture to get all the buffer sizes

	  img = cvQueryFrame(capture);
	  if(!img){
			if (argc == 2){
					printf("End of video file reached\n");
			} else {
					printf("ERROR: cannot get next fram from camera\n");
			}
					exit(1);
	  }

       // allocate all the required buffers

            IplImage* hsv = cvCloneImage(img);
            IplImage* hue = cvCreateImage( cvGetSize(img), 8, 1 );
			hue->origin = img->origin;
            IplImage* mask = cvCloneImage(hue);
            IplImage* backproject = cvCloneImage(hue);
            cvZero(backproject);

		// allocate all required histogram buffers

			int hdims = 16;
			float hranges_arr[] = {0,180};
			float* hranges = hranges_arr;
			CvHistogram* histM = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
			CvHistogram* histL = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
            IplImage* histimgM = cvCreateImage( cvSize(320,200), 8, 3 );
			IplImage* histimgL = cvCreateImage( cvSize(320,200), 8, 3 );
            cvZero( histimgM );
			cvZero( histimgL );

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

		  // *** CAMSIFT Histogram based tracking code

		    // create an HSV version of the input image

			 cvCvtColor( img, hsv, CV_BGR2HSV );

			 // create a mask for all values within S channel value {smin ... 256}
			 // and V channel value {vmin .. vmax}

			cvInRangeS( hsv, cvScalar(0, smin, min(vmin,vmax),0),
                        cvScalar(180, 256, max(vmin,vmax),0), mask );

			// isolate the hue channel

			cvSplit( hsv, hue, NULL, NULL, NULL );

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

			// if we have selected a new template area within the image

			if ( (!(select_object)) && (selection.width > 0) && (selection.height > 0) &&
				 (!histogramCaptured))
			{

				// get the histogram for the Hue channel
				// within the areas defined Saturation and Variance Channel mask

				float max_val = 0.f;
                cvSetImageROI( hue, selection );
                cvSetImageROI( mask, selection );
                cvCalcHist( &hue, histM, 0, mask );
                cvGetMinMaxHistValue( histM, 0, &max_val, 0, 0 );
                cvConvertScale( histM->bins, histM->bins, max_val ? 255. / max_val : 0., 0 );
                cvResetImageROI( hue );
                cvResetImageROI( mask );

				// set the initial tracking position of the object

				track_window = selection;

				// create the histogram image of model

                cvZero( histimgM );
                int bin_w = histimgM->width / hdims;
                for(int i = 0; i < hdims; i++ )
                {
                    int val = cvRound( cvGetReal1D(histM->bins,i)*histimgM->height/255 );
                    CvScalar color = hue2rgb(i*180.f/hdims);
                    cvRectangle( histimgM, cvPoint(i*bin_w,histimgM->height),
                                 cvPoint((i+1)*bin_w,histimgM->height - val),
                                 color, -1, 8, 0 );
                }

				histogramCaptured = true;
			}

			// if we have a selected template perform cross correlation template matching

			if (histogramCaptured){

				// calculate the back-projection of the histogram on the image

				cvCalcBackProject( &hue, backproject, histM );
				cvAnd( backproject, mask, backproject, 0 );

				// pass this backprojection to the camShift Routine

				cvCamShift(backproject, track_window,
                        cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
                        &track_comp, &track_box );

				// update the tracking window for the next iteration based
				// on the output of the current predicted position

				track_window = track_comp.rect;

				// draw the detected result (taking into account image origin)

				if( img->origin ) {
					track_box.angle = -track_box.angle;
				}

				 cvEllipseBox( img, track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );

				// draw the histogram of the detected result area

				float max_val = 0.f;
                cvSetImageROI( hue, track_window );
                cvSetImageROI( mask, track_window );
                cvCalcHist( &hue, histL, 0, mask );
                cvGetMinMaxHistValue( histL, 0, &max_val, 0, 0 );
                cvConvertScale( histL->bins, histL->bins, max_val ? 255. / max_val : 0., 0 );
                cvResetImageROI( hue );
                cvResetImageROI( mask );

				// create the histogram image of model

                cvZero( histimgL );
                int bin_w = histimgL->width / hdims;
                for(int i = 0; i < hdims; i++ )
                {
                    int val = cvRound( cvGetReal1D(histL->bins,i)*histimgL->height/255 );
                    CvScalar color = hue2rgb(i*180.f/hdims);
                    cvRectangle( histimgL, cvPoint(i*bin_w,histimgL->height),
                                 cvPoint((i+1)*bin_w,histimgL->height - val),
                                 color, -1, 8, 0 );
                }

			}

		  // ***

		  // display image in window

		  if (!showBackProjection){
			cvShowImage( windowName, img );
		  } else {
			cvShowImage( windowName, backproject );
		  }
		  cvShowImage( windowName2, histimgM );
		  cvShowImage( windowName3, mask );
		  cvShowImage( windowName4, histimgL );

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

	   		// if user presses "c" then reset histogram model

				printf("\nHistogram model reset by user request\n");

	   			histogramCaptured = false;
				cvZero( histimgM );
				cvZero( histimgL );
				selection.width = 0;
				selection.height = 0;
		  }  else if (key == 'b'){

	   		// if user presses "b" then show backprojection

				printf("\nBack projection view toggled by user request\n");

	   			showBackProjection = (!showBackProjection);
		  }
	  }

      // destroy window objects
      // (triggered by event loop *only* window is closed)

      cvDestroyAllWindows();

      // destroy image objects (if it does not originate from a capture object)

	  cvReleaseImage ( &img_copy);
	  cvReleaseImage ( &backproject );
	  cvReleaseImage ( &mask  );
	  cvReleaseImage ( &hsv );
	  cvReleaseImage ( &hue);
      cvReleaseImage ( &histimgM);
	  cvReleaseImage ( &histimgL);

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
