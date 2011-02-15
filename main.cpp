
#include "Controller.h"

int main (int argc, char * const argv[]) {

	IplImage* loadImg = cvLoadImage("test.jpg");
	Controller controller(loadImg);
	controller.run();

	return 1;

}



