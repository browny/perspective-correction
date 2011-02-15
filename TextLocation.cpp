
#include "TextLocation.h"

const int SMOOTH_SIZE = 3;
const int BOX_MAXNUM = 10;
const float CC_PERIMETER = 50.0f; // connected component perimeter
const double AREA_RATIO = 0.05;
const double HISTO_RATIO = 0.5;
const double GL_TH_BIAS = -10; // global threshold bias

template<class T>
string to_string(T t) {
	ostringstream oss;
	oss << std::dec << t;
	return oss.str();
}

TextLocation::TextLocation(const IplImage* img) {

	boundBoxRect = cvRect(0, 0, 0, 0);
	boundBoxCenter = cvPoint(0, 0);

	CvSize imgSize = cvGetSize(img);

	src      = cvCreateImage(imgSize, img->depth, img->nChannels);
	gray     = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	smooth   = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	edge     = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	globalTh = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	th       = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
	outImg   = cvCreateImage(imgSize, img->depth, img->nChannels);

	cvCopy(img, src);
	cvCopy(img, outImg);
	cvCvtColor(src, gray, CV_RGB2GRAY);

}

void TextLocation::threshold(const IplImage* grayImg, IplImage* th) {

	cvSmooth(grayImg, smooth, CV_GAUSSIAN, SMOOTH_SIZE, SMOOTH_SIZE);
	cvShowImage("smooth", smooth);

	// edge
	cvCanny(smooth, edge, 60, 150, 3);
	cvDilate(edge, edge, NULL, 1);
	cvSaveImage("edge.jpg", edge);

	// global threshold
	double thValue = getHistIdxAtRatio(src, HISTO_RATIO);
	cvThreshold(smooth, globalTh, thValue , 255, 0);
	inverseBinaryImage(globalTh);
	cvSaveImage("global.jpg", globalTh);

	cvAnd(globalTh, edge, th);
	cvDilate(th, th, NULL, 2);

}

void TextLocation::boundBox(const IplImage* th, CvRect& boundBoxRect, CvPoint& boundBoxCenter) {

	vector<CvRect> ccRects;
	vector<CvPoint> ccCenters;
	maxLimitConnectComponet(th, BOX_MAXNUM, ccRects, ccCenters);

	// choose one best cc rect (contains most text)
	CvPoint imgCenter = cvPoint(src->width/2, src->height/2);
	double  imgArea   = src->width * src->height;

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
	cvCircle(outImg, boundBoxCenter, 5, CV_RGB(0, 255, 0), -1);
	cvRectangle(outImg, cvPoint(boundBoxRect.x, boundBoxRect.y), cvPoint(boundBoxRect.x
			+ boundBoxRect.width, boundBoxRect.y + boundBoxRect.height), CV_RGB(0, 255, 0), 2);

	cvShowImage("text boxes", outImg);
	cvSaveImage("box.jpg", outImg);

}

TextLocation::~TextLocation() {

	cvReleaseImage(&src);
	cvReleaseImage(&gray);
	cvReleaseImage(&smooth);
	cvReleaseImage(&edge);
	cvReleaseImage(&globalTh);
	cvReleaseImage(&th);
	cvReleaseImage(&outImg);

}


// --- Private Function --- //

void TextLocation::connectComponent(IplImage* src, const int poly_hull0, const float perimScale,
		int *num, vector<CvRect> &rects, vector<CvPoint> &centers) {

	/*
	 * Pre : "src"        :is the input image
	 *       "poly_hull0" :is usually set to 1
	 *       "perimScale" :defines how big connected component will be retained, bigger
	 *                     the number, more components are retained (100)
	 *
	 * Post: "num"        :defines how many connected component was found
	 *       "rects"      :the bounding box of each connected component
	 *       "centers"    :the center of each bounding box
	 */

	rects.clear();
	centers.clear();

	CvMemStorage* mem_storage = NULL;
	CvSeq* contours = NULL;

	// Clean up
	cvMorphologyEx(src, src, 0, 0, CV_MOP_OPEN, 1);
	cvMorphologyEx(src, src, 0, 0, CV_MOP_CLOSE, 1);

	// Find contours around only bigger regions
	mem_storage = cvCreateMemStorage(0);

	CvContourScanner scanner = cvStartFindContours(src, mem_storage, sizeof(CvContour),
			CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	CvSeq* c;
	int numCont = 0;

	while ((c = cvFindNextContour(scanner)) != NULL) {

		double len = cvContourPerimeter(c);

		// calculate perimeter len threshold
		double q = (double) (src->height + src->width) / perimScale;

		// get rid of blob if its perimeter is too small
		if (len < q) {

			cvSubstituteContour(scanner, NULL);

		} else {

			// smooth its edge if its large enough
			CvSeq* c_new;
			if (poly_hull0) {

				// polygonal approximation
				c_new = cvApproxPoly(c, sizeof(CvContour), mem_storage, CV_POLY_APPROX_DP, 2, 0);

			} else {

				// convex hull of the segmentation
				c_new = cvConvexHull2(c, mem_storage, CV_CLOCKWISE, 1);

			}

			cvSubstituteContour(scanner, c_new);

			numCont++;
		}
	}

	contours = cvEndFindContours(&scanner);

	// Calc center of mass and/or bounding rectangles
	if (num != NULL) {

		// user wants to collect statistics
		int numFilled = 0, i = 0;

		for (i = 0, c = contours; c != NULL; c = c->h_next, i++) {

			if (i < *num) {

				// bounding retangles around blobs

				rects.push_back(cvBoundingRect(c));

				CvPoint center = cvPoint(rects[i].x + rects[i].width / 2, rects[i].y
						+ rects[i].height / 2);
				centers.push_back(center);

				numFilled++;
			}
		}

		*num = numFilled;

	}

	cvReleaseMemStorage(&mem_storage);

}

void TextLocation::inverseBinaryImage(IplImage* img) {

	// turn pixel value from 0 to 255, 255 to 0
	for (int j = 0; j < img->height; j++) {

		uchar* ptr = (uchar*) (img->imageData + j * img->widthStep);

		for (int i = 0; i < img->width; i++) {

			ptr[i] = (ptr[i] == 255) ? 0 : 255;

		}
	}
}

double TextLocation::dist(CvPoint a, CvPoint b) {
	return sqrt(pow((double) (a.x - b.x), 2) + pow((double) (a.y - b.y), 2));
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

void TextLocation::getSubImg(const IplImage* src, const CvRect &roiRect, IplImage* subImg) {

	IplImage* img = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	cvCopy(src, img);

	cvSetImageROI(img, roiRect);
	cvCopy(img, subImg, NULL);
	cvResetImageROI(img);

	cvReleaseImage(&img);

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

void TextLocation::plot1DHisto(const vector<int> &hist, int markIdx) {

	// plot 1D Histogram
	IplImage* imgHistogram = cvCreateImage(cvSize(256, 50), 8, 1);

	cvRectangle(imgHistogram, cvPoint(0, 0), cvPoint(256, 50), CV_RGB(255,255,255), -1);
	int max_value = *(max_element(hist.begin(), hist.end()));

	for (int i = 0; i < 256; ++i) {
		int val = hist[i];
		int nor = cvRound(val * 50 / max_value);
		cvLine(imgHistogram, cvPoint(i, 50), cvPoint(i, 50 - nor), CV_RGB(0,0,0));
	}

	cvLine(imgHistogram, cvPoint(markIdx, 50), cvPoint(markIdx, 0),
				CV_RGB(255,255,255));

	cvShowImage("hist", imgHistogram);

	cvReleaseImage(&imgHistogram);

}

void TextLocation::maxLimitConnectComponet(const IplImage* img, const int maxNum,
		vector<CvRect> &rects, vector<CvPoint> &centers) {

	IplImage* thCopy = cvCreateImage(cvGetSize(th), th->depth, th->nChannels);
	cvCopy(img, thCopy);

	// connected component
	int ccNum = 200;
	connectComponent(thCopy, 1, CC_PERIMETER, &ccNum, rects, centers);

	// too many ccNum, dilate the cc img then cc again
	IplImage* maskImg = cvCreateImage(cvGetSize(th), th->depth, th->nChannels);
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
