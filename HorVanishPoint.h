
#ifndef _HORVANISHPOINT_H_
#define _HORVANISHPOINT_H_

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#define PI 3.141592

const double SEARCH_MIN = 0.5;
const double SEARCH_MAX = 1;

class HorVanishPoint {

public:

	HorVanishPoint(const IplImage* src, CvRect rect);

	// get included angle between 2 vectors
	double includedAngle(CvPoint2D32f vec1, CvPoint2D32f vec2);

	// get spreaded angle enclosing text region rectangle from point
	double spreadedAngleOfRectFromPoint(CvPoint2D32f pt, CvRect rect);

	~HorVanishPoint();


private:

	CvPoint imgCenter;

	double radiusStep; // 0.5 diagonal length of text region rectangle
	double radiusRatio;

	double radius;
	double radian;

};



#endif
