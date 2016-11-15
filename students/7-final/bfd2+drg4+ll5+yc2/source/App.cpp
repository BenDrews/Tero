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

    // VR CODE
    //VRApp::Settings settings(argc, argv);
    //settings.vr.debugMirrorMode = //VRApp::DebugMirrorMode::NONE;//
    //    VRApp::DebugMirrorMode::PRE_DISTORTION;
    //
    //settings.vr.disablePostEffectsIfTooSlow = false;

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

App::App(const AppBase::Settings& settings) : super(settings) {
}

void App::makeGUI() {
    // Initialize the developer HUD
    createDeveloperHUD();

//    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);
 
//    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}

// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be
// automatically caught.
void App::onInit() {
    super::onInit();
    setFrameDuration(1.0f / 120.0f);

    showRenderingStats      = false;

    makeGUI();
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));

    loadScene("G3D Whiteroom");

	initializeScene();
}


void App::initializeScene() {
    m_posToVox = Table<Point3int32, int>();

    m_voxToProp = Table<int, Any>();

    for(int x = -50; x < 50; ++x) {
        for(int z = -50; z < 50; ++z) {
            m_posToVox.set(Point3int32(x, 0, z), 0);
        }
    }

    for(int i = 0; i < voxTypeCount; ++i) {
        m_voxToProp.set(i, Any::fromFile(format("data-files/voxelTypes/vox%d.Any", i)));
    }

	initializeMaterials();

    addVoxelModelToScene();
}

void App::initializeMaterials() {
	m_voxToMat = Table<int, shared_ptr<UniversalMaterial>>();

	for (int i = 0; i < voxTypeCount; ++i) {
		Any any = m_voxToProp.get(i);

		shared_ptr<UniversalMaterial> mat = UniversalMaterial::create(
			PARSE_ANY(
				UniversalMaterial::Specification {
				lambertian = Texture::Specification {
					filename = format("data-files/texture/%s", any[mat]);
				};
				}
			)
		);

		// ???????????????????????????????????????????????????????????????????
		m_voxToMat.set(i, mat);
	}

}

// Create a cube model. Code pulled from sample/proceduralGeometry
void App::addVoxelModelToScene() {
    // Replace any existing voxel model. Models don't 
    // have to be added to the model table to use them 
    // with a VisibleEntity.
    const shared_ptr<Model>& voxelModel = initializeModel();
    if (scene()->modelTable().containsKey(voxelModel->name())) {
        scene()->removeModel(voxelModel->name());
    }
    scene()->insert(voxelModel);

    // Replace any existing voxel entity that has the wrong type
    shared_ptr<Entity> voxel = scene()->entity("voxel");
    if (notNull(voxel) && isNull(dynamic_pointer_cast<VisibleEntity>(voxel))) {
        logPrintf("The scene contained an Entity named 'voxel' that was not a VisibleEntity\n");
        scene()->remove(voxel);
        voxel.reset();
    }

    if (isNull(voxel)) {
        // There is no voxel entity in this scene, so make one.
        //
        // We could either explicitly instantiate a VisibleEntity or simply
        // allow the Scene parser to construct one. The second approach
        // has more consise syntax for this case, since we are using all constant
        // values in the specification.
        voxel = scene()->createEntity("voxel",
            PARSE_ANY(
                VisibleEntity {
                    model = "voxelModel";
                };
            ));
    } else {
        // Change the model on the existing voxel entity
        dynamic_pointer_cast<VisibleEntity>(voxel)->setModel(voxelModel);
    }

    voxel->setFrame(CFrame::fromXYZYPRDegrees(0.0f, 0.0f, 0.0f, 45.0f, 45.0f));
}


void App::addVoxel(Point3int32 input, int type) {
	m_posToVox.set(input, type);

    ArticulatedModel::Geometry* geometry  = m_model->addGeometry(format("geom%d,%d,%d", input.x, input.y, input.z));
    ArticulatedModel::Mesh*     mesh      = m_model->addMesh(format("mesh%d,%d,%d", input.x, input.y, input.z), m_model->part("root"), geometry);

	// Check each position adjacent to voxel, and if nothing is there, add a face
    if ( !m_posToVox.containsKey(input + Vector3int32(1,0,0)) ) {
        addFace(input, Vector3int32(1,0,0), type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(-1,0,0)) ) {
        addFace(input, Vector3int32(-1,0,0), type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,1,0)) ) {
        addFace(input, Vector3int32(0,1,0), type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,-1,0)) ) {
        addFace(input, Vector3int32(0,-1,0), type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,0,1)) ) {
        addFace(input, Vector3int32(0,0,1), type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,0,-1)) ) {
        addFace(input, Vector3int32(0,0,-1), type, geometry, mesh);
    }

}

void App::addFace(Point3int32 input, Vector3 normal, int type, ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh) {

	mesh->material = m_voxToMat.get(type);

}

void App::removeVoxel(Point3int32 input) {

}



shared_ptr<Model> App::initializeModel() {

    ArticulatedModel::Part*     part      = m_model->addPart("root");
    ArticulatedModel::Geometry* geometry  = m_model->addGeometry("geom");
    ArticulatedModel::Mesh*     mesh      = m_model->addMesh("mesh", part, geometry);

    // Assign a material
    mesh->material = UniversalMaterial::create(
        PARSE_ANY(
        UniversalMaterial::Specification {
            lambertian = Texture::Specification {
                filename = "image/checker-32x32-1024x1024.png";
                // Orange
                encoding = Color3(1.0, 0.7, 0.15);
            };

            glossy     = Color4(Color3(0.01), 0.2);
        }));

	// Fill vertex and index arrays
	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>& indexArray = mesh->cpuIndexArray;

//	for (int i = 0; i < 9; ++i) { 
        CPUVertexArray::Vertex& a = vertexArray.next();
	    a.position = Point3(0,0,0);


	    CPUVertexArray::Vertex& b = vertexArray.next();
	    b.position = Point3(10,0,0);						//1


	    CPUVertexArray::Vertex& c = vertexArray.next();
	    c.position = Point3(10,0,-10);						//2


	    CPUVertexArray::Vertex& d = vertexArray.next();
	    d.position = Point3(10,10,-10);						//3
  

	    CPUVertexArray::Vertex& e = vertexArray.next();
	    e.position = Point3(0,10,-10);						//4


	    CPUVertexArray::Vertex& f = vertexArray.next();
	    f.position = Point3(0,10,0);						//5
  

	    CPUVertexArray::Vertex& g = vertexArray.next();
	    g.position = Point3(10,10,0);						//6


	    CPUVertexArray::Vertex& h = vertexArray.next();
	    h.position = Point3(0,0,-10);						//7


//	}
	// Create the indices
//	for (int i = 0; i < 50; ++i) {
	    mesh->cpuIndexArray.append(0, 1, 6, 6, 5, 0);

		mesh->cpuIndexArray.append(1, 2, 3, 3, 6, 1);

	    mesh->cpuIndexArray.append(0, 7, 2, 2, 1, 0);

	    mesh->cpuIndexArray.append(7, 4, 3, 3, 2, 7);

	    mesh->cpuIndexArray.append(4, 7, 0, 0, 5, 4);

        mesh->cpuIndexArray.append(3, 4, 5, 5, 6, 3);
//	}

	// Tell m_model to generate bounding boxes, GPU vertex arrays, normals, and tangents automatically
	ArticulatedModel::CleanGeometrySettings geometrySettings;
    geometrySettings.allowVertexMerging = false;
    m_model->cleanGeometry(geometrySettings);

	return m_model;
}