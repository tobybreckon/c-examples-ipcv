// Example : deinterlace an image
// usage: prog <image_name>

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2008 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include "cv.h"       // opencv general include file
#include "highgui.h"  // opencv GUI include file
#include "cvaux.h"    // opencv AUX include file

#include <stdio.h>
#include <string.h>


/******************************************************************************/

// simple function to insert string before the ".jpg" or similar 
// filename extension

void insertStringBeforeExtension(char * filename, char * stringToInsert){
	
	char *dotLocation = strpbrk(filename, ".");
	char extenstionPart[strlen(filename) - strlen(dotLocation)]; 
	
	strcpy(extenstionPart, dotLocation);
	*dotLocation = '\0';
	strcat(filename, stringToInsert);
	strcat(filename, extenstionPart);
}

/******************************************************************************/

int main( int argc, char** argv )
{

  IplImage* img = NULL;      // image object
  IplImage *oddImage, *evenImage = NULL;      // odd/even image object	
	
  char filename [FILENAME_MAX];	
	
  // if command line arguments are provided try to read image/video_name
  // otherwise default to capture from attached H/W camera 

    if( 
	  ( argc == 2 && (img = cvLoadImage( argv[1], CV_LOAD_IMAGE_UNCHANGED)) != 0 ) 
	  )
    {
	  
	  // create output image
	  
	  oddImage = cvCreateImage(cvSize(img->width,(img->height / 2)), 
						   img->depth, img->nChannels);
	  oddImage->origin = img->origin;		
	  evenImage = cvCreateImage(cvSize(img->width, (img->height / 2)), 
						   img->depth, img->nChannels);
	  evenImage->origin = img->origin;	
		
	  // de-interlace image
	  
	  cvDeInterlace(img, evenImage, oddImage);
	  
	  // resize even image to original input size
	  
	  cvResize(evenImage, img, CV_INTER_CUBIC);

	  // write to file

	  strcpy(filename, argv[1]);
	  insertStringBeforeExtension(filename, "EVENFRAME");
	  cvSaveImage(filename, img);

      // all OK : main returns 0

      return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/
