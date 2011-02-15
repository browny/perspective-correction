
#ifndef _HOMOGRAPHY_H_
#define _HOMOGRAPHY_H_

#include <cv.h>
#include <cxcore.h>
#include <vector>
using namespace std;
using namespace cv;

class Homography {

public:

	Homography();

	void getHomoMat(vector<CvPoint> pts, int width, int height);
	void perspectiveCorrect(const IplImage* src, IplImage* dst);

private:

	Mat homoMat; // homography matrix

};

#endif /* _HOMOGRAPHY_H_ */
