
#ifndef _UTILITY_H_
#define _UITLITY_H_

#include <vector>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
using namespace std;

double dist(CvPoint a, CvPoint b);

void inverseBinaryImage(IplImage* img);

void connectComponent(IplImage* src, const int poly_hull0, const float perimScale, int *num,
		vector<CvRect> &rects, vector<CvPoint> &centers);

void getSubImg(const IplImage* src, const CvRect &roiRect, IplImage* subImg);

void plot1DHisto(const vector<int> &hist, int markIdx);

#endif
