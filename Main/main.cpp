#include <dvtxTest.h>
#include <bindableTest.h>
#include <testConstant.h>
#include <modeltest.h>
#include <pbr.h>
#include <stb_image.h>

int main() {
	init();

	RTREffects::PBR_IBL app("PBR");
	//ModelTest app("scene");
	//TestConstant app;
	app.start();

	fin();

	return 0;
}