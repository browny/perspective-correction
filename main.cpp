
#include "Controller.h"

int main (int argc, char * const argv[]) {

	IplImage* loadImg = cvLoadImage("test.jpg");

	// Initialization controller
	Controller controller(loadImg, "Win");
	controller.setupCallbacks();

	// run
	controller.run();

	return 1;

}



