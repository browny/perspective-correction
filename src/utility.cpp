
#include "utility.h"

double dist(CvPoint a, CvPoint b) {
	return sqrt(pow((double) (a.x - b.x), 2) + pow((double) (a.y - b.y), 2));
}


void inverseBinaryImage(IplImage* img) {

	// turn pixel value from 0 to 255, 255 to 0
	for (int j = 0; j < img->height; j++) {

		uchar* ptr = (uchar*) (img->imageData + j * img->widthStep);

		for (int i = 0; i < img->width; i++) {

			ptr[i] = (ptr[i] == 255) ? 0 : 255;

		}
	}
}

void connectComponent(IplImage* src, const int poly_hull0, const float perimScale, int *num,
		vector<CvRect> &rects, vector<CvPoint> &centers) {

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


void getSubImg(const IplImage* src, const CvRect &roiRect, IplImage* subImg) {

	IplImage* img = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	cvCopy(src, img);

	cvSetImageROI(img, roiRect);
	cvCopy(img, subImg, NULL);
	cvResetImageROI(img);

	cvReleaseImage(&img);

}

void plot1DHisto(const vector<int> &hist, int markIdx) {

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


