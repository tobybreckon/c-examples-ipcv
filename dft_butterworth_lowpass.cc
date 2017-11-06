// Example : apply butterworth low pass filtering to input image/video
// usage: prog {<image_name> | <video_name>}

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2009 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

// portions based on OpenCV library example dft.c

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>

#include <algorithm> // contains max() function (amongst others)
using namespace cv; // use c++ namespace so the timing stuff works consistently
using namespace std;

/******************************************************************************/
// Rearrange the quadrants of Fourier image so that the origin is at
// the image center
// src & dst arrays of equal size & type
void cvShiftDFT(CvArr * src_arr, CvArr * dst_arr );

/******************************************************************************/
// setup the camera index / file capture properly based on OS platform

// 0 in linux gives first camera for v4l
//-1 in windows gives first device or user dialog selection

#ifdef linux
	#define CAMERA_INDEX 0
	#define VIDEOCAPTURE cvCaptureFromFile
#else
	#define CAMERA_INDEX -1
    #define VIDEOCAPTURE cvCaptureFromAVI
#endif
/******************************************************************************/

// return a floating point spectrum magnitude image scaled for user viewing

// dft_A - input dft (2 channel floating point, Real + Imaginary fourier image)
// rearrange - perform rearrangement of DFT quadrants if > 0
// return value - pointer to output spectrum magnitude image scaled for user viewing

IplImage* create_spectrum_magnitude_display(CvMat* dft_A, int rearrange)
{

	double m, M;
    IplImage* image_Re = cvCreateImage( cvSize(dft_A->cols, dft_A->rows), IPL_DEPTH_64F, 1);
    IplImage* image_Im = cvCreateImage( cvSize(dft_A->cols, dft_A->rows), IPL_DEPTH_64F, 1);

	 // Split Fourier in real and imaginary parts
	  cvSplit( dft_A, image_Re, image_Im, 0, 0 );

	 // Compute the magnitude of the spectrum Mag = sqrt(Re^2 + Im^2)
	 cvPow( image_Re, image_Re, 2.0);
	 cvPow( image_Im, image_Im, 2.0);
	 cvAdd( image_Re, image_Im, image_Re, NULL);
	 cvPow( image_Re, image_Re, 0.5 );

	 // Compute log(1 + Mag)
	 cvAddS( image_Re, cvScalarAll(1.0), image_Re, NULL ); // 1 + Mag
	 cvLog( image_Re, image_Re ); // log(1 + Mag)

	// Rearrange the quadrants of Fourier image so that the origin is at
	// the image center
	if (rearrange){
		cvShiftDFT( image_Re, image_Re );
	}

	 // scale image for display
	 cvMinMaxLoc(image_Re, &m, &M, NULL, NULL, NULL);
	 cvScale(image_Re, image_Re, 1.0/(M-m), 1.0*(-m)/(M-m));

	 // release imaginary image part
	 cvReleaseImage(&image_Im);

	// return DFT spectrum

	return image_Re;

}
/******************************************************************************/

// create a 2-channel butterworth low-pass filter with radius D, order n

void create_butterworth_lowpass_filter(CvMat* dft_Filter, int D, int n)
{
	CvMat* single = cvCreateMat(dft_Filter->rows, dft_Filter->cols, CV_64FC1 );

	CvPoint centre = cvPoint(dft_Filter->rows / 2, dft_Filter->cols / 2);
	double radius;

	// based on the forumla in the IP notes (p. 130 of 2009/10 version)
	// see also HIPR2 on-line

	for(int i = 0; i < dft_Filter->rows; i++)
	{
		for(int j = 0; j < dft_Filter->cols; j++)
		{
			radius = (double) sqrt(pow((i - centre.x), 2.0) + pow((double) (j - centre.y), 2.0));
			CV_MAT_ELEM(*single, double, i, j) =
						( 1 / (1 + pow((double) (radius /  D), (double) (2 * n))));
		}
	}

	cvMerge(single, single, NULL, NULL, dft_Filter);

	cvReleaseMat(&single);
}


/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object
  CvCapture* capture = NULL; // capture object

  IplImage* dft_spec_mag = NULL;

  char const * originalName = "Original Image (grayscale)"; // window name
  char const * lowPassName = "Butterworth Low Pass Filtered (grayscale)"; // window name
  char const * spectrumMagName = "Magnitude Image (log transformed)"; // window name
  char const * filterName = "Filter Image"; // window name

  bool keepProcessing = true;	// loop control flag
  char key;						// user input
  int  EVENT_LOOP_DELAY = 40;	// delay for GUI window
                                // 40 ms equates to 1000ms/25fps = 40ms per frame

  int radius = 30;				// low pass filter parameter
  int order = 2;				// low pass filter parameter

  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera

    if(
	  ( argc == 2 && (img = cvLoadImage( argv[1], CV_LOAD_IMAGE_UNCHANGED)) != 0 ) ||
	  ( argc == 2 && (capture = VIDEOCAPTURE( argv[1] )) != 0 ) ||
	  ( argc != 2 && (capture = cvCreateCameraCapture( 0 )) != 0 )
	  )
    {
      // create window objects (use flag=0 to allow resize, 1 to auto fix size)

      cvNamedWindow(originalName, 0);
	  cvNamedWindow(lowPassName, 0);
	  cvNamedWindow(spectrumMagName, 0);
	  cvNamedWindow(filterName, 0);

	  // define required floating point images for DFT processing
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

	  // do setup for required DFT images and arrays

      IplImage* realInput = cvCreateImage( cvGetSize(img), IPL_DEPTH_64F, 1);
      IplImage* imaginaryInput = cvCreateImage( cvGetSize(img), IPL_DEPTH_64F, 1);
      IplImage* complexInput = cvCreateImage( cvGetSize(img), IPL_DEPTH_64F, 2);

	  int dft_M = cvGetOptimalDFTSize( img->height - 1 );
      int dft_N = cvGetOptimalDFTSize( img->width - 1 );

      CvMat* dft_A = cvCreateMat( dft_M, dft_N, CV_64FC2 );
	  CvMat* dft_Filter = cvCreateMat( dft_M, dft_N, CV_64FC2 );

	  CvMat tmp;
	  double m, M;
      IplImage* image_Re = cvCreateImage( cvSize(dft_N, dft_M), IPL_DEPTH_64F, 1);
	  IplImage* image_Re_Filter = cvCreateImage( cvGetSize(dft_Filter), IPL_DEPTH_64F, 1);
      IplImage* image_Im = cvCreateImage( cvSize(dft_N, dft_M), IPL_DEPTH_64F, 1);

	  // add adjustable trackbar for low pass filter threshold parameter

      cvCreateTrackbar("Radius", lowPassName, &radius, (min(dft_M, dft_N) / 2), NULL);
	  cvCreateTrackbar("Order", lowPassName, &order, 10, NULL);

	  // define grayscale image

	  IplImage* grayImg =
	  			cvCreateImage(cvSize(img->width,img->height), img->depth, 1);
	  grayImg->origin = img->origin;

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

		  }	//else {


		  	  // THIS SECTION IS COMMENTED OUT TO ALLOW FOR INTERACTIVE
		  	  // PARAMETER CHANGES ON A SINGLE IMAGE

			  // if not a capture object set event delay to zero so it waits
			  // indefinitely (as single image file, no need to loop)

			  // EVENT_LOOP_DELAY = 0;
		  //}

		  // *** Fourier processing

		  	  // if input is not already grayscale, convert to grayscale

			  if (img->nChannels > 1){
				  cvCvtColor(img, grayImg, CV_BGR2GRAY);
			  } else {
				  grayImg = img;
			  }

		  // convert grayscale image to real part of DTF input

		  cvScale(grayImg, realInput, 1.0, 0.0);
    	  cvZero(imaginaryInput);
    	  cvMerge(realInput, imaginaryInput, NULL, NULL, complexInput);

		  // copy A to dft_A and pad dft_A with zeros

		  cvGetSubRect( dft_A, &tmp, cvRect(0,0, grayImg->width,grayImg->height));
		  cvCopy( complexInput, &tmp, NULL );
		  cvGetSubRect( dft_A, &tmp, cvRect(img->width,0, dft_A->cols - grayImg->width, grayImg->height));
		  if ((dft_A->cols - grayImg->width) > 0)
		  {
		  	cvZero( &tmp );
		  }

		  // set up filter (first channel is real part / second is imaginary

		  create_butterworth_lowpass_filter(dft_Filter, radius, order);

		  // no need to pad bottom part of dft_A with zeros because of
		  // use nonzero_rows parameter in cvDFT() call below

		  cvDFT( dft_A, dft_A, CV_DXT_FORWARD, complexInput->height );
		  cvShiftDFT( dft_A, dft_A );

		  // apply filter

		  cvMulSpectrums( dft_A, dft_Filter, dft_A, 0);
		  cvShiftDFT( dft_A, dft_A );

		  // compute spectrum magnitude for display

		  if (dft_spec_mag != NULL){
			  cvReleaseImage(&dft_spec_mag);
		  }
		  dft_spec_mag = create_spectrum_magnitude_display(dft_A, 1);

		  // invert dft

		  cvDFT( dft_A, dft_A, CV_DXT_INVERSE, complexInput->height );

		  // Split Fourier in real and imaginary parts
		  cvSplit( dft_A, image_Re, image_Im, 0, 0 );

         // scale image for display
		 cvMinMaxLoc(image_Re, &m, &M, NULL, NULL, NULL);
         cvScale(image_Re, image_Re, 1.0/(M-m), 1.0*(-m)/(M-m));

		 image_Re->origin = grayImg->origin;

		 // prepare an image of the filter for display
		 // i.e. copy and scale for display

		  cvSplit(dft_Filter, image_Re_Filter, NULL, NULL, NULL);
		  cvMinMaxLoc(image_Re_Filter, &m, &M, NULL, NULL, NULL);
		  cvScale(image_Re_Filter, image_Re_Filter, 1.0/(M-m), 1.0*(-m)/(M-m));

		  // display image in window

		  cvShowImage( originalName, grayImg );
		  cvShowImage( lowPassName, image_Re ); // N.B. floating point image
		  cvShowImage( spectrumMagName, dft_spec_mag );
		  cvShowImage( filterName, image_Re_Filter); // N.B. floating point image

		  // start event processing loop (very important,in fact essential for GUI)
	      // 4 ms roughly equates to 100ms/25fps = 4ms per frame

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

	  // release other images

	  cvReleaseMat( &dft_A);
	  cvReleaseMat( &dft_Filter);
	  if (grayImg) {cvReleaseImage( &grayImg );}
	  cvReleaseImage( &realInput );
	  cvReleaseImage( &imaginaryInput );
	  cvReleaseImage( &complexInput );
	  cvReleaseImage( &image_Re );
	  cvReleaseImage( &image_Im );
	  cvReleaseImage( &dft_spec_mag );
	  cvReleaseImage ( &image_Re_Filter );

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
// Rearrange the quadrants of Fourier image so that the origin is at
// the image center
// src & dst arrays of equal size & type
void cvShiftDFT(CvArr * src_arr, CvArr * dst_arr )
{
    CvMat * tmp = NULL;
    CvMat q1stub, q2stub;
    CvMat q3stub, q4stub;
    CvMat d1stub, d2stub;
    CvMat d3stub, d4stub;
    CvMat * q1, * q2, * q3, * q4;
    CvMat * d1, * d2, * d3, * d4;

    CvSize size = cvGetSize(src_arr);
    CvSize dst_size = cvGetSize(dst_arr);
    int cx, cy;

    if(dst_size.width != size.width ||
       dst_size.height != size.height){
        cvError( CV_StsUnmatchedSizes,
		   "cvShiftDFT", "Source and Destination arrays must have equal sizes",
		   __FILE__, __LINE__ );
    }

    if(src_arr==dst_arr){
        tmp = cvCreateMat(size.height/2, size.width/2, cvGetElemType(src_arr));
    }

    cx = size.width/2;
    cy = size.height/2; // image center

    q1 = cvGetSubRect( src_arr, &q1stub, cvRect(0,0,cx, cy) );
    q2 = cvGetSubRect( src_arr, &q2stub, cvRect(cx,0,cx,cy) );
    q3 = cvGetSubRect( src_arr, &q3stub, cvRect(cx,cy,cx,cy) );
    q4 = cvGetSubRect( src_arr, &q4stub, cvRect(0,cy,cx,cy) );
    d1 = cvGetSubRect( src_arr, &d1stub, cvRect(0,0,cx,cy) );
    d2 = cvGetSubRect( src_arr, &d2stub, cvRect(cx,0,cx,cy) );
    d3 = cvGetSubRect( src_arr, &d3stub, cvRect(cx,cy,cx,cy) );
    d4 = cvGetSubRect( src_arr, &d4stub, cvRect(0,cy,cx,cy) );

    if(src_arr!=dst_arr){
        if( !CV_ARE_TYPES_EQ( q1, d1 )){
            cvError( CV_StsUnmatchedFormats,
			"cvShiftDFT", "Source and Destination arrays must have the same format",
			__FILE__, __LINE__ );
        }
        cvCopy(q3, d1, 0);
        cvCopy(q4, d2, 0);
        cvCopy(q1, d3, 0);
        cvCopy(q2, d4, 0);
    }
    else{
        cvCopy(q3, tmp, 0);
        cvCopy(q1, q3, 0);
        cvCopy(tmp, q1, 0);
        cvCopy(q4, tmp, 0);
        cvCopy(q2, q4, 0);
        cvCopy(tmp, q2, 0);

		cvReleaseMat(&tmp);
    }
}
/******************************************************************************/
