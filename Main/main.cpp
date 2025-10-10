#include <dvtxTest.h>
#include <bindableTest.h>
#include <testConstant.h>
#include <modeltest.h>
#include <pbr.h>
#include <OIT.h>
#include <stb_image.h>
#include <Defer.h>
#include <Pipeline/defer.h>

int main() {
	init();

	// RTREffects::PBR_IBL app("PBR");
	// RTREffects::OIT app("OIT");
	// ModelTest app("scene");
	// TestConstant app;
	// RTREffects::DeferRenderer app("Defer_Renderer");
	OGLPipeline::DeferRenderer app("Defer Renderer");
	app.start();

	fin();

	return 0;
}