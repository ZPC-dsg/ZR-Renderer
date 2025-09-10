#include <dvtxTest.h>
#include <bindableTest.h>
#include <testConstant.h>
#include <modeltest.h>
#include <pbr.h>
#include <OIT.h>
#include <stb_image.h>
#include <Defer.h>

int main() {
	init();

	// RTREffects::PBR_IBL app("PBR");
	// RTREffects::OIT app("OIT");
	// ModelTest app("scene");
	// TestConstant app;
	RTREffects::DeferRenderer app("Defer_Renderer");
	app.start();

	fin();

	return 0;
}