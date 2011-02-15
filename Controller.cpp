
#include "Controller.h"
#include "Homography.h"
#include "TextLocation.h"
#include "HorVanishPoint.h"

Controller::Controller(const IplImage* img) {

	ctrlPtIdx = -1;

	src       = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	srcBackup = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	out       = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	cvCopy(img, src);
	cvCopy(img, srcBackup);

	initCorners(cvGetSize(src));

	cvNamedWindow("Win", 1);
	cvSetMouseCallback("Win", &Controller::onMouse, this);

}

void Controller::run() {

	TextLocation txt(src);
	txt.threshold(txt.gray, txt.th);
	txt.boundBox(txt.th, txt.boundBoxRect, txt.boundBoxCenter);

	drawCorners(src);
	Homography homo;

	// Event loops
	while (1) {

		cvShowImage("Win", src);

		// Keyboard event
		int c = cvWaitKey(30);

		if (c == 'h' || c == 'H') {

			homo.getHomoMat(corners, src->width, src->height);
			homo.perspectiveCorrect(src, out);
			cvShowImage("H", out);

		}

		if ((char) c == 27) { // 'Esc' to terminate

			cvDestroyAllWindows();
			cvReleaseImage(&out);
			exit(1);

			break;
		}

	}

}

void Controller::initCorners(CvSize sz) {

	corners.resize(4);
	int width  = src->width;
	int height = src->height;

	vector<CvPoint>::iterator it = corners.begin();
	while (it != corners.end()) {

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

	vector<CvPoint>::iterator it = corners.begin();
	while (it != corners.end()) {
		cvCircle(img, *it, 5, CV_RGB(255, 0, 0), -2, CV_AA, 0);
		it++;
	}

}

int Controller::getNearestPointIndex(CvPoint mousePt) {

	CvPoint pt;
	for (int i = 0; i < 4; i++) {

		pt.x = mousePt.x - (int) corners[i].x;
		pt.y = mousePt.y - (int) corners[i].y;
		float distance = sqrt((float) (pt.x * pt.x + pt.y * pt.y));
		if (distance < 20)
			return i;
	}

	return -1;
}

void Controller::onMouse(int event, int x, int y, int flags, void *param) {

	// 圖片超過螢幕大小，控制會出錯

	Controller* temp= (Controller*) param;
	CvPoint pt = cvPoint(x, y);

	switch (event) {

		case CV_EVENT_LBUTTONDOWN:

		if (temp->ctrlPtIdx > -1) {
			temp->ctrlPtIdx = -1;
		} else {
			temp->ctrlPtIdx = temp->getNearestPointIndex(pt);
		}

		break;

		case CV_EVENT_MOUSEMOVE:

		if (temp->ctrlPtIdx > -1) {

			temp->corners[temp->ctrlPtIdx].x = x;
			temp->corners[temp->ctrlPtIdx].y = y;

			cvCopy(temp->srcBackup, temp->src);
			temp->drawCorners(temp->src);

		}

		break;

		default:
			break;
	}

}


Controller::~Controller() {

	cvReleaseImage(&src);
	cvReleaseImage(&srcBackup);
	cvReleaseImage(&out);

}
