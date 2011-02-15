
#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <vector>

using namespace std;

class Controller {
public:

	Controller(const IplImage* img);

	void run();

	~Controller();

private:

	int ctrlPtIdx;
	vector<CvPoint> corners;

	IplImage* src;
	IplImage* srcBackup;
	IplImage* out;

	void initCorners(CvSize sz); // initilaize 4 corners
	void drawCorners(IplImage* img);
	int getNearestPointIndex(CvPoint mousePt);

	static void onMouse(int event, int x, int y, int flags, void *param);


};



#endif
