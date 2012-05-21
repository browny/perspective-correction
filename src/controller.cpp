
#include "controller.h"
#include "homography.h"
#include "textLocation.h"
#include "horVanishPoint.h"

Controller::Controller(const IplImage* img, string winName) :
	UIWindow(winName, cvGetSize(img)) {

	m_ctrlPtIdx = -1;

	m_src = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	m_srcBackup = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	m_out = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	cvCopy(img, m_src);
	cvCopy(img, m_srcBackup);

	initCorners(cvGetSize(m_src));

}

void Controller::run() {

	TextLocation txtLocation(m_src);
	txtLocation.threshold(txtLocation.grayImg, txtLocation.thImg);
	txtLocation.boundBox(txtLocation.thImg, txtLocation.boundBoxRect, txtLocation.boundBoxCenter);

	drawCorners(m_src);

	Homography homo;
	// Event loops
	while (1) {

		cvShowImage("Win", m_src);

		// Keyboard event
		int c = cvWaitKey(30);

		if (c == 'h' || c == 'H') {

			homo.getHomoMat(m_cornerList, m_src->width, m_src->height);
			homo.perspectiveCorrect(m_src, m_out);
			cvShowImage("H", m_out);

		}

		if ((char) c == 27) { // 'Esc' to terminate

			cvDestroyAllWindows();
			cvReleaseImage(&m_out);
			exit(1);

			break;
		}

	}

}

void Controller::initCorners(CvSize sz) {

	m_cornerList.resize(4);
	int width  = m_src->width;
	int height = m_src->height;

	vector<CvPoint>::iterator it = m_cornerList.begin();
	while (it != m_cornerList.end()) {

		it->x = width / 10;
		it->y = height / 10;
		it++;

		it->x = width * 9 / 10;
		it->y = height / 10;
		it++;

		it->x = width / 10;
		it->y = height * 9 / 10;
		it++;

		it->x = width * 9 / 10;
		it->y = height * 9 / 10;
		it++;

	}

}

void Controller::drawCorners(IplImage* img) {

	vector<CvPoint>::iterator it = m_cornerList.begin();
	while (it != m_cornerList.end()) {
		cvCircle(img, *it, 5, CV_RGB(255, 0, 0), -2, CV_AA, 0);
		it++;
	}

}

int Controller::getNearestPointIndex(CvPoint mousePt) {

	CvPoint pt;
	for (int i = 0; i < 4; i++) {

		pt.x = mousePt.x - (int) m_cornerList[i].x;
		pt.y = mousePt.y - (int) m_cornerList[i].y;
		float distance = sqrt((float) (pt.x * pt.x + pt.y * pt.y));
		if (distance < 20)
			return i;
	}

	return -1;
}

void Controller::onMouse(int event, int x, int y, int flags, void *param) {

	Controller* pController = (Controller*) param;
	CvPoint pt = cvPoint(x, y);

	switch (event) {

		case CV_EVENT_LBUTTONDOWN:

		if (pController->m_ctrlPtIdx > -1) {
			pController->m_ctrlPtIdx = -1;
		} else {
			pController->m_ctrlPtIdx = pController->getNearestPointIndex(pt);
		}

		break;

		case CV_EVENT_MOUSEMOVE:

		if (pController->m_ctrlPtIdx > -1) {

			pController->m_cornerList[pController->m_ctrlPtIdx].x = x;
			pController->m_cornerList[pController->m_ctrlPtIdx].y = y;

			cvCopy(pController->m_srcBackup, pController->m_src);
			pController->drawCorners(pController->m_src);

		}

		break;

		default:
			break;
	}

}

void Controller::setupCallbacks() {

	cvSetMouseCallback(cvGetWindowName(m_window), &Controller::onMouse, this);

}

void Controller::onMouseCallback(int event, int x, int y) {

}

void Controller::onKeyCallback(int keyCode) {

}


Controller::~Controller() {

	cvReleaseImage(&m_src);
	cvReleaseImage(&m_srcBackup);
	cvReleaseImage(&m_out);

}
