
#include <math.h>
#include <vector>
#include "HorVanishPoint.h"

using namespace std;

HorVanishPoint::HorVanishPoint(const IplImage* src, CvRect rect) {

	imgCenter = cvPoint(src->width/2, src->height/2);

	radiusStep = 0.5 * sqrt(pow(rect.width, 2) + pow(rect.height, 2));
	radiusRatio = SEARCH_MIN;

	radius = radiusRatio * radiusStep;
	radian = 0;

}

double HorVanishPoint::includedAngle(CvPoint2D32f vec1, CvPoint2D32f vec2) {

	const double epsilon = 1.0e-6;
	double angle = 0;

	// normalize
	CvPoint2D32f norVec1, norVec2;
	norVec1.x = vec1.x / sqrt(pow(vec1.x, 2) + pow(vec1.y, 2));
	norVec1.y = vec1.y / sqrt(pow(vec1.x, 2) + pow(vec1.y, 2));
	norVec2.x = vec2.x / sqrt(pow(vec2.x, 2) + pow(vec2.y, 2));
	norVec2.y = vec2.y / sqrt(pow(vec2.x, 2) + pow(vec2.y, 2));

	// dot product
	double dotProd = (norVec1.x * norVec2.x) + (norVec1.y * norVec2.y);
	if (abs(dotProd - 1.0) <= epsilon)
		angle = 0;
	else if (abs(dotProd + 1.0) <= epsilon)
		angle = PI;
	else {
		angle = acos(dotProd);
	}

	return abs(angle * (180 / PI));

}

double HorVanishPoint::spreadedAngleOfRectFromPoint(CvPoint2D32f pt, CvRect rect) {

	// the coordinate of pt is relative to the center of rect

	CvPoint2D32f p1, p2, p3, p4;
	p1 = cvPoint2D32f(-rect.width/2, -rect.height/2);
	p2 = cvPoint2D32f( rect.width/2, -rect.height/2);
	p3 = cvPoint2D32f(-rect.width/2,  rect.height/2);
	p4 = cvPoint2D32f( rect.width/2,  rect.height/2);

	vector<CvPoint2D32f> vecs;
	vecs.resize(4, cvPoint2D32f(0, 0));

	vecs[0] = cvPoint2D32f(p1.x - pt.x, p1.y - pt.y);
	vecs[1] = cvPoint2D32f(p2.x - pt.x, p2.y - pt.y);
	vecs[2] = cvPoint2D32f(p3.x - pt.x, p3.y - pt.y);
	vecs[3] = cvPoint2D32f(p4.x - pt.x, p4.y - pt.y);

	CvPoint2D32f upBoundVec = cvPoint2D32f(0, 0);
	CvPoint2D32f downBoundVec = cvPoint2D32f(0, 0);
	double angle = 0;

	for (unsigned int i = 0; i < vecs.size(); ++i) {
		for (unsigned int j = i+1; j < vecs.size(); ++j) {

			if ( includedAngle(vecs[i], vecs[j]) > angle ) {

				angle = includedAngle(vecs[i], vecs[j]);
				upBoundVec = vecs[i];
				downBoundVec = vecs[j];

			}

		}
	}

	return angle;

}


HorVanishPoint::~HorVanishPoint() {

}

