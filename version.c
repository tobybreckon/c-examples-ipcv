// Example : test OpenCV version
// usage: prog
// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // open cv general include file

#include <stdio.h>

/******************************************************************************/

int main( int argc, char** argv )
{

printf ("You are using OpenCV version %s (%d.%d.%d)\n",
	    CV_VERSION,
	    CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION);
scanf  (" ");
}
/******************************************************************************/
