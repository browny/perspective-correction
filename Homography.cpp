
#include "homography.h"

Homography::Homography() {

	m_homoMat.create(3, 3, CV_32FC1);

}

void Homography::getHomoMat(vector<CvPoint> pts, int width, int height) {

	float ptsArr[4 * 2];
	for (int i = 0; i < 4; i++) {

		ptsArr[i*2]     = (float) pts[i].x;
		ptsArr[i*2 + 1] = (float) pts[i].y;

	}

	float mapArr[] = { 0,     0,
			           width, 0,
			           0,     height,
			           width, height };

	Mat coords_mat1(4, 2, CV_32FC1, ptsArr);
	Mat coords_mat2(4, 2, CV_32FC1, mapArr);

	m_homoMat = findHomography(coords_mat1, coords_mat2, 0);

}

void Homography::perspectiveCorrect(const IplImage* src, IplImage* dst) {

	Mat srcMat(src);
	Mat dstMat(dst);

	cv::warpPerspective(srcMat, dstMat, m_homoMat, cvGetSize(dst), INTER_LINEAR, BORDER_CONSTANT, 0);

	*dst = dstMat;

}
