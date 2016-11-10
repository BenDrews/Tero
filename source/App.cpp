/**
	G3D App that launches a GLSL shader
*/
#include "App.h"

App::App(const GApp::Settings& settings) : GApp(settings) {
}

void App::onInit() {
    GApp::onInit();

    setFrameDuration(1.0f / 60.0f);
    showRenderingStats  = false;

    createDeveloperHUD();
    developerWindow->sceneEditorWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->setVisible(false);
    developerWindow->videoRecordDialog->setEnabled(true);

    Texture::Specification e;
    e.filename = System::findDataFile("cubemap/noonclouds/noonclouds_*.png");
    e.encoding.format = ImageFormat::SRGB8();
    m_environmentMap = Texture::create(e);

	// Turn off bloom, antiAliasing, vignetting
	debugCamera()->filmSettings().setBloomStrength(0.0f);
	debugCamera()->filmSettings().setAntialiasingEnabled(false);
	debugCamera()->filmSettings().setVignetteTopStrength(0.0f);
    debugCamera()->setFieldOfViewAngle(50 * units::degrees());
}

void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    // Bind the main framebuffer
    rd->push2D(m_framebuffer); {
        rd->clear();

        // Set the arguments for the shader that will be launched
        Args args;

        args.setUniform("cameraToWorldMatrix", activeCamera()->frame());

        m_environmentMap->setShaderArgs(args, "environmentMap.", Sampler::cubeMap());
        args.setUniform("environmentMap_MIPConstant", std::log2(float(m_environmentMap->width() * sqrt(3.0f))));

        args.setUniform("tanHalfFieldOfViewY",  float(tan(activeCamera()->projection().fieldOfViewAngle() / 2.0f)));

        // Projection matrix for writing to depth buffer
		// This creates the input that allows us to get a depth of field effect.
        Matrix4 projectionMatrix;
        activeCamera()->getProjectUnitMatrix(rd->viewport(), projectionMatrix);
        args.setUniform("projectionMatrix22", projectionMatrix[2][2]);
        args.setUniform("projectionMatrix23", projectionMatrix[2][3]);

        // Set the domain of the shader to the viewport rectangle
        args.setRect(rd->viewport());

		// For every pixel in the domain, do the program in raymarcher.pix
		// This call returns immediately on the CPU and the code executes asynchronously
		// on the GPU.
        LAUNCH_SHADER(
			//"raymarcher-correctness.pix"
			"raymarcher.pix"
			, args);

        // Post-process special effects
        m_depthOfField->apply(rd, m_framebuffer->texture(0), 
                              m_framebuffer->texture(Framebuffer::DEPTH), 
                              activeCamera(), Vector2int16());
        
    } rd->pop2D();

    swapBuffers();

    rd->clear();

    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0), 0, 0);
}

G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    GApp::Settings settings(argc, argv);

    settings.window.caption = "Simple GPU Ray Tracer";
    settings.window.width   = 848;
    settings.window.height  = 480;
    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(0, 0);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
	settings.screenshotDirectory = "../journal/";
	settings.dataDir = FileSystem::currentDirectory() + "/data-files/";

    return App(settings).run();
}

