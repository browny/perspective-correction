
#ifndef _TEXTLOCATION_H_
#define _TEXTLOCATION_H_

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <vector>

using namespace std;

class TextLocation {

public:

	TextLocation(const IplImage* img);

	CvRect  boundBoxRect;
	CvPoint boundBoxCenter;

	IplImage* gray;
	IplImage* th;

	void threshold(const IplImage* grayImg, IplImage* th);
	void boundBox(const IplImage* th, CvRect& boundBoxRect, CvPoint& boundBoxCenter);

	~TextLocation();

private:

	IplImage* src;
	IplImage* smooth;
	IplImage* edge;
	IplImage* globalTh;
	IplImage* outImg;

	void connectComponent(IplImage* src, const int poly_hull0, const float perimScale, int *num,
			vector<CvRect> &rects, vector<CvPoint> &centers);

	void inverseBinaryImage(IplImage* img);

	double dist(CvPoint a, CvPoint b);

	void getSubImg(const IplImage* src, const CvRect &roiRect, IplImage* subImg);

	double getTxtRatioInRect(const IplImage* src, const CvRect &roi);

	double getHistIdxAtRatio(const IplImage* src, double ratio);

	void getMaskImgFromRects(const IplImage* src, const vector<CvRect> &rects, IplImage* dst);

	void plot1DHisto(const vector<int> &hist, int markIdx);

	void maxLimitConnectComponet(const IplImage* img, const int maxNum, vector<CvRect> &rects,
			vector<CvPoint> &centers);

};


#endif
