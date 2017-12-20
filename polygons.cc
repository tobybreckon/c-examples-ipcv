// Example : using openCV polygon functions

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2010 School of Engineering, Cranfield University
// License : GPL - http://www.gnu.org/licenses/gpl.html

#include "cv.h"       // open cv general include file
#include "highgui.h"  // open cv GUI include file

#include <stdio.h>

/******************************************************************************/

int main( int argc, char** argv )
{
	// create an image (set it to a black background)

  	IplImage* img = cvCreateImage(cvSize(400, 400), IPL_DEPTH_8U, 3);
  	cvZero(img);


	// define a polygon

	int npts = 6; 	// number of polygon points
	int npolys = 1; // number of polygons to draw with cvPolyLine()

	CvPoint pts[6];	// array of points (1 per polygon)
	CvPoint* polys[1] = {pts};	// array of polygon pt arrays

	pts[0].x = 50; pts[0].y = 50;
	pts[1].x = 300; pts[1].y = 50;
	pts[2].x = 350; pts[2].y = 200;
	pts[3].x = 300; pts[3].y = 150;
	pts[4].x = 150; pts[4].y = 350;
	pts[5].x = 100; pts[5].y = 100;

	// draw the polygon

	cvPolyLine( img, polys, &npts, npolys,
	    		1, 				// draw closed contour (i.e. joint end to start)
	            CV_RGB(0,255,0),// colour RGB ordering (here = green)
	    		3, 				// line thickness
	    		CV_AA, 			// line type (Anti-Aliased)
	    		0 );			// shift=0


	// do point in polygon test (first convert it to an array of points)

	CvPoint2D32f test_pt;
	CvMat *polyForTest = cvCreateMat(npts, 1, CV_32SC2);
	for (int i = 0; i < npts; i++)
	{
		CV_MAT_ELEM(*polyForTest, CvPoint, i, 0) = pts[i];
	}

	// define and test point one (draw it in red)

	test_pt.x = 150;
	test_pt.y = 75;
	cvRectangle(img, cvPointFrom32f(test_pt),cvPointFrom32f(test_pt),
	    		  CV_RGB(255, 0, 0), 3, 8, 0);
	if (cvPointPolygonTest(polyForTest, test_pt, 1) > 0){
		printf("RED {%f, %f} is in the polygon (dist. %f)\n",
		    					test_pt.x, test_pt.y,
		    					cvPointPolygonTest(polyForTest, test_pt, 1));
	}

	// define and test point two (draw it in blue)

	test_pt.x = 50;
	test_pt.y = 350;
	cvRectangle(img, cvPointFrom32f(test_pt),cvPointFrom32f(test_pt),
	    		  CV_RGB(0, 0, 255), 3, 8, 0);
	if (cvPointPolygonTest(polyForTest, test_pt, 1) < 0){
		printf("BLUE {%f, %f} is NOT in the polygon (dist. %f)\n",
		    					test_pt.x, test_pt.y,
		    					cvPointPolygonTest(polyForTest, test_pt, 1));
	}

	// Beware the OpenCV manual (1.0) entry for cvPointPolygonTest() should read
	// "When measure_dist=0, the return value is +100, -100 and 0, respectively.
	// When measure_dist≠0, it is a signed distance between the point and the
	// nearest contour edge." - not +1, -1 and 0 (!).

	// create an image and display the image

  	cvNamedWindow("Polygon Test", 0);
	cvShowImage( "Polygon Test", img );
	cvWaitKey(0);

    return 0;
}

/******************************************************************************/
