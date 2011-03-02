
#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__

#include <cxcore.h>
#include <highgui.h>
#include <string>
using namespace std;

class UIWindow {
public:

	void* m_window;
	IplImage* m_windowImg;

	virtual void setupCallbacks() = 0;
	virtual void onMouseCallback(int event, int x, int y) = 0;
	virtual void onKeyCallback(int keyCode) = 0;

	virtual ~UIWindow() {

		cvReleaseImage(&m_windowImg);
		cvDestroyWindow(cvGetWindowName(m_window));
	};

protected:

	UIWindow(string winName, CvSize winSize) {

		cvNamedWindow(winName.c_str(), 1);
		cvResizeWindow(winName.c_str(), winSize.width, winSize.height);

		m_window = cvGetWindowHandle(winName.c_str());
		m_windowImg = cvCreateImage(winSize, IPL_DEPTH_8U, 3);

	}


};

#endif
