#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;

	ofSetupOpenGL(&window, 800, 600, OF_WINDOW);
	ofRunApp(new testApp());
}
