/*
 * test_slic.cpp.
 *
 * Written by: Pascal Mettes.
 *
 * This file creates an over-segmentation of a provided image based on the SLIC
 * superpixel algorithm, as implemented in slic.h and slic.cpp.
 */
 
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <float.h>

#include "slic.h"
#include "slic.cpp"

using namespace std;


int main(int argc, char *argv[]) {
    /* Load the image and convert to Lab colour space. */
    char* file_path = "dog.png";
    Image lab_image(file_path);
    //cvCvtColor(image, lab_image, CV_BGR2Lab);
    
    /* Yield the number of superpixels and weight-factors from the user. */
    int nr_superpixels = atoi(argv[1]);
    int nc = atoi(argv[2]);
    

    double step = sqrt((lab_image.width * lab_image.height) / (double) nr_superpixels);
    
    cout << nr_superpixels << " " << nc << endl;
    cout << lab_image.channels << " " << lab_image.width << " " << lab_image.height << endl;

    /* Perform the SLIC superpixel algorithm. */
    Slic slic;
    slic.generate_superpixels(&lab_image, step, nc);
    cout << "end super" << endl;
    slic.create_connectivity(&lab_image);
    cout << "end conect" << endl;
    /* Display the contours and show the result. */

    slic.display_contours(&lab_image, Color(255,0,0));
    lab_image.write("result.png");

}
