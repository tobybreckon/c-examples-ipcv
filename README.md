# C Image Processing and Computer Vision OpenCV Teaching Examples

OpenCV C Interface Image Processing and Computer Vision legacy examples used for teaching, instruction and reference over the years (2006-2010).

_Uses C interface to OpenCV, with additional C++ code - as these examples pre-date the C++ OpenCV interface - available post OpenCV 2.x_

All tested with OpenCV 2.x and GCC (Linux) and known to work with MS Visual Studio 200x on Win32 / Win64.
N.B. due to changes in the OpenCV API _these do not generically work with OpenCV > 2.x_ by default.

(see https://github.com/tobybreckon/cpp-examples-ipcv for OpenCV 3.x C++ examples).

---

### Background:

If I taught you between 2006 and 2010+ at [Cranfield University](http://www.cranfield.ac.uk) or [ESTIA](http://www.estia.fr) - these are the examples from class.

Additionally used to generate the video examples within the ebook version of:

[Dictionary of Computer Vision and Image Processing](http://dx.doi.org/10.1002/9781119286462) (R.B. Fisher, T.P. Breckon, K. Dawson-Howe, A. Fitzgibbon, C. Robertson, E. Trucco, C.K.I. Williams), Wiley, 2014.
[[Google Books](http://books.google.co.uk/books?id=TaEQAgAAQBAJ&lpg=PP1&dq=isbn%3A1118706811&pg=PP1v=onepage&q&f=false)] [[doi](http://dx.doi.org/10.1002/9781119286462)]

---

### How to Build and run:

```
git clone https://github.com/tobybreckon/c-examples-ipcv.git
cd c-examples-ipcv
cmake .
make
./<insert executable name of one of the examples>
```

Demo source code is provided _"as is"_ to aid your learning and understanding -- see my textbook http://www.fundipbook.com for the theory (and matlab code also).

Most run with a webcam connected or from a command line supplied video file of a format OpenCV supports on your system (otherwise edit the script to provide your own image source).

N.B. you may need to change the line near the top that specifies the camera device to use on some examples below - change "0" if you have one webcam, I have it set to "1" to skip my built-in laptop webcam and use the connected USB camera.

---

### Reference:

All techniques are fully explained in corresponding section of:

_Fundamentals of Digital Image Processing: A Practical Approach with Examples in Matlab_,
Chris J. Solomon and Toby P. Breckon, Wiley-Blackwell, 2010
ISBN: 0470844736, DOI:10.1002/9780470689776, http://www.fundipbook.com

```
bibtex:
@Book{solomonbreckon10fundamentals,
  author 	= 	 {Solomon, C.J. and Breckon, T.P.},
  title 	= 	 {Fundamentals of Digital Image Processing:
                                A Practical Approach with Examples in Matlab},
  publisher 	= 	 {Wiley-Blackwell},
  year 		= 	 {2010},
  isbn 		= {0470844736},
  doi 		= {10.1002/9780470689776},
  url 		= {http://www.fundipbook.com}
}
```

---

If you find any bugs report them to me (or better still submit a pull request, please) - toby.breckon@durham.ac.uk

_"may the source be with you"_ - anon.
