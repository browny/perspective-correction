
#include "textLocation.h"
#include "utility.h"

const int SMOOTH_SIZE = 3;
const int BOX_MAXNUM = 10;
const float CC_PERIMETER = 50.0f; // connected component perimeter
const double AREA_RATIO = 0.05;
const double HISTO_RATIO = 0.5;
const double GL_TH_BIAS = -10; // global threshold bias

TextLocation::TextLocation(const IplImage* img) {

	boundBoxRect = cvRect(0, 0, 0, 0);
	boundBoxCenter = cvPoint(0, 0);

	CvSize imgSize = cvGetSize(img);

	grayImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	thImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);

	m_src = cvCreateImage(imgSize, img->depth, img->nChannels);
	m_smooth = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	m_edge = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	m_globalTh = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	m_outImg = cvCreateImage(imgSize, img->depth, img->nChannels);

	cvCopy(img, m_src);
	cvCopy(img, m_outImg);
	cvCvtColor(m_src, grayImg, CV_RGB2GRAY);

}

void TextLocation::threshold(const IplImage* grayImg, IplImage* th) {

	cvSmooth(grayImg, m_smooth, CV_GAUSSIAN, SMOOTH_SIZE, SMOOTH_SIZE);
	cvShowImage("smooth", m_smooth);

	// edge
	cvCanny(m_smooth, m_edge, 60, 150, 3);
	cvDilate(m_edge, m_edge, NULL, 1);
	cvSaveImage("edge.jpg", m_edge);

	// global threshold
	double thValue = getHistIdxAtRatio(m_src, HISTO_RATIO);
	cvThreshold(m_smooth, m_globalTh, thValue , 255, 0);
	inverseBinaryImage(m_globalTh);
	cvSaveImage("global.jpg", m_globalTh);

	cvAnd(m_globalTh, m_edge, th);
	cvDilate(th, th, NULL, 2);

}

void TextLocation::boundBox(const IplImage* th, CvRect& boundBoxRect, CvPoint& boundBoxCenter) {

	vector<CvRect> ccRects;
	vector<CvPoint> ccCenters;
	maxLimitConnectComponet(th, BOX_MAXNUM, ccRects, ccCenters);

	// choose one best cc rect (contains most text)
	CvPoint imgCenter = cvPoint(m_src->width/2, m_src->height/2);
	double  imgArea   = m_src->width * m_src->height;

	double  ratio = 1;

	for (unsigned int i = 0; i < ccCenters.size(); ++i) {

		double ccArea = ccRects[i].width * ccRects[i].height;

		if (ccArea > AREA_RATIO*imgArea) {

			/*cvCircle(outImg, ccCenters[i], 5, CV_RGB(255, 255, 0), -1);
			cvRectangle(outImg, cvPoint(ccRects[i].x, ccRects[i].y), cvPoint(ccRects[i].x
					+ ccRects[i].width, ccRects[i].y + ccRects[i].height), CV_RGB(255, 255, 0), 2);*/

			double nonTxtRatio = 1.0 - getTxtRatioInRect(th, ccRects[i]);

			if (nonTxtRatio < ratio) {

				ratio = nonTxtRatio;
				boundBoxCenter = ccCenters[i];
				boundBoxRect = ccRects[i];

			}
		}
	}

	// draw selected cc rect
	cvCircle(m_outImg, boundBoxCenter, 5, CV_RGB(0, 255, 0), -1);
	cvRectangle(m_outImg, cvPoint(boundBoxRect.x, boundBoxRect.y), cvPoint(boundBoxRect.x
			+ boundBoxRect.width, boundBoxRect.y + boundBoxRect.height), CV_RGB(0, 255, 0), 2);

	cvShowImage("text boxes", m_outImg);
	cvSaveImage("box.jpg", m_outImg);

}

TextLocation::~TextLocation() {

	cvReleaseImage(&grayImg);
	cvReleaseImage(&thImg);

	cvReleaseImage(&m_src);
	cvReleaseImage(&m_smooth);
	cvReleaseImage(&m_edge);
	cvReleaseImage(&m_globalTh);
	cvReleaseImage(&m_outImg);

}

double TextLocation::getTxtRatioInRect(const IplImage* src, const CvRect &roi) {

	IplImage* subImg = cvCreateImage(cvSize(roi.width, roi.height), src->depth, src->nChannels);
	getSubImg(src, roi, subImg);

	// calculate the white pixel ratio of assigned roi in src image
	double whitePixelSum = 0;
	for (int row = 0; row < subImg->height; row++) {

		uchar* pSubImg = (uchar*) (subImg->imageData + row * subImg->widthStep);

		for (int col = 0; col < subImg->width; col++) {

			if (pSubImg[col] == 255)
				whitePixelSum++;

		}
	}

	double ratio = whitePixelSum / (subImg->width * subImg->height);
	cvReleaseImage(&subImg);

	return ratio;

}


void TextLocation::getMaskImgFromRects(const IplImage* src, const vector<CvRect> &rects, IplImage* dst) {

	// use rects to mask src image
	IplImage* mask = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
    cvZero(mask);

    for (unsigned int i = 0; i < rects.size(); ++i) {

    	for (int row = rects[i].y; row < rects[i].y + rects[i].height; ++row) {

    		uchar* pMask = (uchar*) (mask->imageData + row * mask->widthStep);

    		for (int col = rects[i].x; col < rects[i].x + rects[i].width; ++col) {

    			pMask[col] = 255;

    		}
    	}
    }

    IplImage* zeroImg = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	cvZero(zeroImg);
	cvOr(src, zeroImg, dst, mask);

	cvReleaseImage(&mask);
	cvReleaseImage(&zeroImg);

}

double TextLocation::getHistIdxAtRatio(const IplImage* src, double ratio) {

	// image pixel value histogram
	vector<int> hist(256, 0);
	for (int j = 0; j < src->height; j++) {
		uchar* pSrc = (uchar*) (src->imageData + j * src->widthStep);
		for (int i = 0; i < src->width; i++) {
				hist[pSrc[i]] += 1;
		}
	}

	// decide which bin accumulation from 255 sum up to HISTO_RATIO
	double acc = 0;
	double thresholdPixelCount = ratio * (src->width * src->height);

	int threshold = 255;
	while (acc < thresholdPixelCount) {
		acc += hist[threshold];
		threshold--;
	}

	threshold += GL_TH_BIAS;

	// plot histogram
	plot1DHisto(hist, threshold);

	return threshold;


}

void TextLocation::maxLimitConnectComponet(const IplImage* img, const int maxNum,
		vector<CvRect> &rects, vector<CvPoint> &centers) {

	IplImage* thCopy = cvCreateImage(cvGetSize(thImg), thImg->depth, thImg->nChannels);
	cvCopy(img, thCopy);

	// connected component
	int ccNum = 200;
	connectComponent(thCopy, 1, CC_PERIMETER, &ccNum, rects, centers);

	// too many ccNum, dilate the cc img then cc again
	IplImage* maskImg = cvCreateImage(cvGetSize(thImg), thImg->depth, thImg->nChannels);
	cvCopy(img, thCopy);

	while (ccNum > BOX_MAXNUM) {

		getMaskImgFromRects(thCopy, rects, maskImg);
		cvErode(maskImg, maskImg, NULL, 1);
		cvDilate(maskImg, maskImg, NULL, 2);

		cvSaveImage("maskImg.jpg", maskImg);

		cvCopy(maskImg, thCopy);
		connectComponent(maskImg, 1, CC_PERIMETER, &ccNum, rects, centers);

	}

	cvReleaseImage(&maskImg);
	cvReleaseImage(&thCopy);

}
