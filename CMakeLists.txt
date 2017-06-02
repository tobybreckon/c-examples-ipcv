# CMakeLists file for all examples except from:

# displayimage.cc
# liveimage.cc
# edgedemo.cc
# writevideo.cc
# colourquery.cc
# smoothimage.cc
# saveimage.cc
# livevideo.cc
# displayimage.cc

cmake_minimum_required (VERSION 2.6)

# force .cc files to use c++ compiler if needed
# set_source_files_properties(adaptive_threshold.cc PROPERTIES LANGUAGE CXX )

set( CMAKE_CXX_FLAGS "-O3 -Wall ${CMAKE_CXX_FLAGS}" )

# SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "CMakeCache.txt;CMakeFiles;cmake_install.ccmake")

# linux specific stuff

IF ( UNIX )
   set( CMAKE_PREFIX_PATH "/opt/opencv-2.4" )
   set( CMAKE_CXX_FLAGS "-m64 -Ofast -flto -march=native -funroll-loops ${CMAKE_CXX_FLAGS}" )
   set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
   MESSAGE( "LINUX CONFIG" )
ENDIF ( UNIX )

# windows (inc. 64-bit specific stuff)

IF ( WIN32 )
   set( CMAKE_PREFIX_PATH "C:/OpenCV2.4/build" )
   set( OpenCV_DIR "C:/OpenCV2.4/build" )
   MESSAGE( "WINDOWS CONFIG" )
ENDIF ( WIN32 )

find_package( OpenCV REQUIRED )

project(adaptive_threshold)
add_executable(adaptive_threshold adaptive_threshold.cc)
target_link_libraries( adaptive_threshold ${OpenCV_LIBS} )

project(camshift_tracking)
add_executable(camshift_tracking camshift_tracking.cc)
target_link_libraries( camshift_tracking ${OpenCV_LIBS} )

project(canny)
add_executable(canny canny.cc)
target_link_libraries( canny ${OpenCV_LIBS} )

project(changedetection)
add_executable(changedetection changedetection.cc)
target_link_libraries( changedetection ${OpenCV_LIBS} )

project(colourshape)
add_executable(colourshape colourshape.cc)
target_link_libraries( colourshape ${OpenCV_LIBS} )

project(compactness)
add_executable(compactness compactness.cc)
target_link_libraries( compactness ${OpenCV_LIBS} )

project(connected_components)
add_executable(connected_components connected_components.cc)
target_link_libraries( connected_components ${OpenCV_LIBS} )

project(contraststretch)
add_executable(contraststretch contraststretch.cc)
target_link_libraries( contraststretch ${OpenCV_LIBS} )

project(deinterlace)
add_executable(deinterlace deinterlace.cc)
target_link_libraries( deinterlace ${OpenCV_LIBS} )

project(dft_bandpass)
add_executable(dft_bandpass dft_bandpass.cc)
target_link_libraries( dft_bandpass ${OpenCV_LIBS} )

project(dft_butterworth_highpass)
add_executable(dft_butterworth_highpass dft_butterworth_highpass.cc)
target_link_libraries( dft_butterworth_highpass ${OpenCV_LIBS} )

project(dft_butterworth_lowpass)
add_executable(dft_butterworth_lowpass dft_butterworth_lowpass.cc)
target_link_libraries( dft_butterworth_lowpass ${OpenCV_LIBS} )

project(dft_highpass)
add_executable(dft_highpass dft_highpass.cc)
target_link_libraries( dft_highpass ${OpenCV_LIBS} )

project(dft_homomorphic)
add_executable(dft_homomorphic dft_homomorphic.cc)
target_link_libraries( dft_homomorphic ${OpenCV_LIBS} )

project(dft_lowpass)
add_executable(dft_lowpass dft_lowpass.cc)
target_link_libraries( dft_lowpass ${OpenCV_LIBS} )

project(edge_based_segmentation)
add_executable(edge_based_segmentation edge_based_segmentation.cc)
target_link_libraries( edge_based_segmentation ${OpenCV_LIBS} )

project(eigenimage_based_recognition)
add_executable(eigenimage_based_recognition eigenimage_based_recognition.cc)
target_link_libraries( eigenimage_based_recognition ${OpenCV_LIBS} )

project(erode_dilate)
add_executable(erode_dilate erode_dilate.cc)
target_link_libraries( erode_dilate ${OpenCV_LIBS} )

project(exponential)
add_executable(exponential exponential.cc)
target_link_libraries( exponential ${OpenCV_LIBS} )

project(face_detection)
add_executable(face_detection face_detection.cc)
target_link_libraries( face_detection ${OpenCV_LIBS} )

project(fgbg_segmentation)
add_executable(fgbg_segmentation fgbg_segmentation.cc)
target_link_libraries( fgbg_segmentation ${OpenCV_LIBS} )

project(fourier)
add_executable(fourier fourier.cc)
target_link_libraries( fourier ${OpenCV_LIBS} )

project(gaussian)
add_executable(gaussian gaussian.cc)
target_link_libraries( gaussian ${OpenCV_LIBS} )

project(generic_interface)
add_executable(generic_interface generic_interface.cc)
target_link_libraries( generic_interface ${OpenCV_LIBS} )

project(histogram_based_recognition)
add_executable(histogram_based_recognition histogram_based_recognition.cc)
target_link_libraries( histogram_based_recognition ${OpenCV_LIBS} )

project(histogram_based_recognition_colour)
add_executable(histogram_based_recognition_colour histogram_based_recognition_colour.cc)
target_link_libraries( histogram_based_recognition_colour ${OpenCV_LIBS} )

project(histogram)
add_executable(histogram histogram.cc)
target_link_libraries( histogram ${OpenCV_LIBS} )

project(histogramequalize)
add_executable(histogramequalize histogramequalize.cc)
target_link_libraries( histogramequalize ${OpenCV_LIBS} )

project(histogramequalize_colour)
add_executable(histogramequalize_colour histogramequalize_colour.cc)
target_link_libraries( histogramequalize_colour ${OpenCV_LIBS} )

project(hough_circles)
add_executable(hough_circles hough_circles.cc)
target_link_libraries( hough_circles ${OpenCV_LIBS} )

project(hough_lines)
add_executable(hough_lines hough_lines.cc)
target_link_libraries( hough_lines ${OpenCV_LIBS} )

project(hsvviewer)
add_executable(hsvviewer hsvviewer.cc)
target_link_libraries( hsvviewer ${OpenCV_LIBS} )

project(jpeg_artefacts)
add_executable(jpeg_artefacts jpeg_artefacts.cc)
target_link_libraries( jpeg_artefacts ${OpenCV_LIBS} )

project(labviewer)
add_executable(labviewer labviewer.cc)
target_link_libraries( labviewer ${OpenCV_LIBS} )

project(laplace)
add_executable(laplace laplace.cc)
target_link_libraries( laplace ${OpenCV_LIBS} )

project(logarithmic)
add_executable(logarithmic logarithmic.cc)
target_link_libraries( logarithmic ${OpenCV_LIBS} )

project(median)
add_executable(median median.cc)
target_link_libraries( median ${OpenCV_LIBS} )

project(moment_based_recognition)
add_executable(moment_based_recognition moment_based_recognition.cc)
target_link_libraries( moment_based_recognition ${OpenCV_LIBS} )

project(moment_orientation)
add_executable(moment_orientation moment_orientation.cc)
target_link_libraries( moment_orientation ${OpenCV_LIBS} )

project(moments)
add_executable(moments moments.cc)
target_link_libraries( moments ${OpenCV_LIBS} )

project(open_close)
add_executable(open_close open_close.cc)
target_link_libraries( open_close ${OpenCV_LIBS} )

project(orientated_gradient)
add_executable(orientated_gradient orientated_gradient.cc)
target_link_libraries( orientated_gradient ${OpenCV_LIBS} )

project(polygons)
add_executable(polygons polygons.cc)
target_link_libraries( polygons ${OpenCV_LIBS} )

project(rgbviewer)
add_executable(rgbviewer rgbviewer.cc)
target_link_libraries( rgbviewer ${OpenCV_LIBS} )

project(rotate_zoom)
add_executable(rotate_zoom rotate_zoom.cc)
target_link_libraries( rotate_zoom ${OpenCV_LIBS} )

project(sobel)
add_executable(sobel sobel.cc)
target_link_libraries( sobel ${OpenCV_LIBS} )

project(template_matching)
add_executable(template_matching template_matching.cc)
target_link_libraries( template_matching ${OpenCV_LIBS} )

project(threshold)
add_executable(threshold threshold.cc)
target_link_libraries( threshold ${OpenCV_LIBS} )

project(threshold_channels)
add_executable(threshold_channels threshold_channels.cc)
target_link_libraries( threshold_channels ${OpenCV_LIBS} )

project(unsharp)
add_executable(unsharp unsharp.cc)
target_link_libraries( unsharp ${OpenCV_LIBS} )

project(version)
add_executable(version version.cc)
target_link_libraries( version ${OpenCV_LIBS} )

project(watershed_segmentation)
add_executable(watershed_segmentation watershed_segmentation.cc)
target_link_libraries( watershed_segmentation ${OpenCV_LIBS} )

project(ycrcbviewer)
add_executable(ycrcbviewer ycrcbviewer.cc)
target_link_libraries( ycrcbviewer ${OpenCV_LIBS} )
