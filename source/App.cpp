/** \file App.cpp */
#include "App.h"

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);

    settings.window.caption             = argv[0];
    settings.window.width               = 1280; settings.window.height       = 720; settings.window.fullScreen          = false;
    settings.window.resizable           = ! settings.window.fullScreen;
    settings.window.framed              = ! settings.window.fullScreen;
    settings.window.asynchronous        = false;

    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(64, 64);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir                    = FileSystem::currentDirectory();
    settings.screenshotDirectory        = "../journal/";

    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = false;

    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {
}

// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be
// automatically caught.
void App::onInit() {
    GApp::onInit();
    setFrameDuration(1.0f / 120.0f);

    showRenderingStats      = false;

    makeGUI();
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));

    loadScene("G3D Triangle");

	shared_ptr<ArticulatedModel> model = ArticulatedModel::createEmpty("voxels");
	ArticulatedModel::Part*     part      = model->addPart("vox");
	ArticulatedModel::Geometry* geometry  = model->addGeometry("geom");
	ArticulatedModel::Mesh*     mesh      = model->addMesh("mesh", part, geometry);
	// Create the vertices
//	for (int i = 0; i < 9; ++i) {
	    CPUVertexArray::Vertex& v = geometry->cpuVertexArray.vertex.next();
	    v.position = Point3(0,0,0);													//0
	    v = geometry->cpuVertexArray.vertex.next();
	    v.position = Point3(10,0,0);												//1
	    v = geometry->cpuVertexArray.vertex.next();
	    v.position = Point3(10,0,-10);												//2
	    v = geometry->cpuVertexArray.vertex.next();
	    v.position = Point3(10,10,-10);												//3
	    v = geometry->cpuVertexArray.vertex.next();
	    v.position = Point3(0,10,-10);												//4
	    v = geometry->cpuVertexArray.vertex.next();
	    v.position = Point3(0,10,0);												//5
	    v = geometry->cpuVertexArray.vertex.next();
	    v.position = Point3(10,10,0);												//6
	    v = geometry->cpuVertexArray.vertex.next();
	    v.position = Point3(0,0,-10);												//7
//	}
	// Create the indices
//	for (int i = 0; i < 50; ++i) {
	    mesh->cpuIndexArray.append(0);
	    mesh->cpuIndexArray.append(1);
	    mesh->cpuIndexArray.append(6);
	    mesh->cpuIndexArray.append(5);

	    mesh->cpuIndexArray.append(1);
	    mesh->cpuIndexArray.append(2);
	    mesh->cpuIndexArray.append(3);
	    mesh->cpuIndexArray.append(6);

	    mesh->cpuIndexArray.append(0);
	    mesh->cpuIndexArray.append(7);
	    mesh->cpuIndexArray.append(2);
	    mesh->cpuIndexArray.append(1);

	    mesh->cpuIndexArray.append(7);
	    mesh->cpuIndexArray.append(4);
	    mesh->cpuIndexArray.append(3);
	    mesh->cpuIndexArray.append(2);

	    mesh->cpuIndexArray.append(4);
	    mesh->cpuIndexArray.append(7);
	    mesh->cpuIndexArray.append(0);
	    mesh->cpuIndexArray.append(5);

	    mesh->cpuIndexArray.append(3);
	    mesh->cpuIndexArray.append(4);
	    mesh->cpuIndexArray.append(5);
	    mesh->cpuIndexArray.append(6);

		mesh->clearIndexStream();
//	}
	// Tell Articulated model to generate normals automatically
	model->cleanGeometry();

	shared_ptr<Scene> scene = GApp::scene();
	scene->insert(model);
}


void onAfterLoadScene(const Any& any, const String& sceneName) {
	

}


void App::makeGUI() {
    // Initialize the developer HUD
    createDeveloperHUD();

//    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);
 
//    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}