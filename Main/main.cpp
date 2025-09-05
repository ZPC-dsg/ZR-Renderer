#include <dvtxTest.h>
#include <bindableTest.h>
#include <testConstant.h>
#include <modeltest.h>
#include <pbr.h>
#include <OIT.h>
#include <stb_image.h>

int main() {
	init();

	// RTREffects::PBR_IBL app("PBR");
	OITEffects::OIT app("OIT");
	// ModelTest app("scene");
	// TestConstant app;
	app.start();

	fin();

	return 0;
}