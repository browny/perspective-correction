
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
	IplImage* grayImg;
	IplImage* thImg;

	void threshold(const IplImage* grayImg, IplImage* th);
	void boundBox(const IplImage* th, CvRect& boundBoxRect, CvPoint& boundBoxCenter);

	~TextLocation();

private:

	IplImage* m_src;
	IplImage* m_smooth;
	IplImage* m_edge;
	IplImage* m_globalTh;
	IplImage* m_outImg;

	double getTxtRatioInRect(const IplImage* src, const CvRect &roi);

	double getHistIdxAtRatio(const IplImage* src, double ratio);

	void getMaskImgFromRects(const IplImage* src, const vector<CvRect> &rects, IplImage* dst);

	void maxLimitConnectComponet(const IplImage* img, const int maxNum, vector<CvRect> &rects,
			vector<CvPoint> &centers);

};


#endif
