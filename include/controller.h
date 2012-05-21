
#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <vector>
#include <string>
#include "uiWindow.h"

using namespace std;

class Controller : public UIWindow {
public:

	Controller(const IplImage* img, string winName);

	// Virtual function implementation
	void setupCallbacks();
	void onMouseCallback(int event, int x, int y);
	void onKeyCallback(int keyCode);

	void run();

	~Controller();

private:

	int m_ctrlPtIdx;
	vector<CvPoint> m_cornerList;

	IplImage* m_src;
	IplImage* m_srcBackup;
	IplImage* m_out;

	void initCorners(CvSize sz); // initilaize 4 corners
	void drawCorners(IplImage* img);
	int getNearestPointIndex(CvPoint mousePt);

	static void onMouse(int event, int x, int y, int flags, void *param);


};



#endif
