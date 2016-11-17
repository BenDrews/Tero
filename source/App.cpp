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

PlayerCamera::PlayerCamera() : position(CFrame(Point3(0,0,0))), speed(0), headTilt(0), heading(0), desiredYaw(0), desiredPitch(0), desiredOS(Vector3(0,0,0)){}


App::App(const AppBase::Settings& settings) : super(settings), crossHair(BoxShape(0.1,0.1,0.1)) {
}

void App::makeGUI() {
    // Initialize the developer HUD
    createDeveloperHUD();

    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);
 
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));

    debugPane->beginRow(); {
        GuiPane* containerPane = debugPane->addPane();
	    
        containerPane->addButton("Add voxel", [this](){
			addVoxel(Point3int32(1,1,1), 0);
	    });
		containerPane->pack();
	}

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

    loadScene("G3D Cornell Box");

	initializeScene();
    addVoxel(Point3int32(1,1,1), 0);
}


void App::initializeScene() {
    m_posToVox = Table<Point3int32, int>();

    m_voxToProp = Table<int, Any>();

	//// Initialize ground
 //   for(int x = -50; x < 50; ++x) {
 //       for(int z = -50; z < 50; ++z) {
 //           m_posToVox.set(Point3int32(x, 0, z), 0);
 //       }
 //   }

    for(int i = 0; i < voxTypeCount; ++i) {
        m_voxToProp.set(i, Any::fromFile(format("voxelTypes/vox%d.Any", i)));
    }

	initializeMaterials();

    addVoxelModelToScene();

    initializePlayer();
}

void App::initializePlayer(){
}

void App::initializeMaterials() {
	m_voxToMat = Table<int, shared_ptr<UniversalMaterial>>();

	for (int i = 0; i < voxTypeCount; ++i) {
		Any any = m_voxToProp.get(i);
		String materialName = any["material"];

		// ???????????????????????????????????????????????????????????????????
		m_voxToMat.set(i, UniversalMaterial::create(
			PARSE_ANY(
				UniversalMaterial::Specification {
				lambertian = Texture::Specification {
					filename = "texture/grass.png";
				};
				}
			)
		));
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
        dynamic_pointer_cast<VisibleEntity>(voxel)->setModel(m_model);
    }

    //voxel->setFrame(CFrame::fromXYZYPRDegrees(0.0f, 0.0f, 0.0f, 45.0f, 45.0f));
}

// Input = Center of vox
void App::addVoxel(Point3int32 input, int type) {
	m_posToVox.set(input, type);

    ArticulatedModel::Geometry* geometry  = m_model->addGeometry(format("geom%d,%d,%d", input.x, input.y, input.z));
    ArticulatedModel::Mesh*     mesh      = m_model->addMesh(format("mesh%d,%d,%d", input.x, input.y, input.z), m_model->part("root"), geometry);

	// Check each position adjacent to voxel, and if nothing is there, add a face
    if ( !m_posToVox.containsKey(input + Vector3int32(1,0,0)) ) {
        addFace(input, Vector3int32(1,0,0), Vector3::X_AXIS, type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(-1,0,0)) ) {
        addFace(input, Vector3int32(-1,0,0), Vector3::X_AXIS, type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,1,0)) ) {
        addFace(input, Vector3int32(0,1,0), Vector3::Y_AXIS, type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,-1,0)) ) {
        addFace(input, Vector3int32(0,-1,0), Vector3::Y_AXIS, type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,0,1)) ) {
        addFace(input, Vector3int32(0,0,1), Vector3::Z_AXIS, type, geometry, mesh);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,0,-1)) ) {
        addFace(input, Vector3int32(0,0,-1), Vector3::Z_AXIS, type, geometry, mesh);
    }

}



void App::addFace(Point3 input, Vector3 normal, Vector3::Axis axis, int type, ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh) {
	mesh->material = m_voxToMat.get(type);

	// Center of face we are adding
	Point3 center = input + normal * 0.5f;

    float sign = normal[axis];
    Vector3 u = Vector3::zero();
    Vector3 v = Vector3::zero();
    u[(axis + 1) % 3] = 1;
    v[(axis + 2) % 3] = 1;

	// Fill vertex and index arrays
	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	//AttributeArray<Vector3> normalArray = geometry->gpuNormalArray;

	Array<int>& indexArray = mesh->cpuIndexArray;
	int index = vertexArray.size();

    CPUVertexArray::Vertex& a = vertexArray.next();
	a.position = (center + u * 0.5f - v * 0.5f) * voxelRes;
    //a.texCoord0=Point2(getTexCoord(a.position,axis));
    a.texCoord0=Point2(1,0);
    a.normal  = Vector3::nan();
    a.tangent = Vector4::nan();

	CPUVertexArray::Vertex& b = vertexArray.next();
	b.position = (center + u * 0.5f + v * 0.5f) * voxelRes;
    //b.texCoord0=Point2(getTexCoord(b.position,axis));
    b.texCoord0=Point2(1,1);
    b.normal  = Vector3::nan();
    b.tangent = Vector4::nan();

	CPUVertexArray::Vertex& c = vertexArray.next();
	c.position = (center - u * 0.5f + v * 0.5f) * voxelRes;
    //c.texCoord0=Point2(getTexCoord(c.position,axis));
    c.texCoord0=Point2(0,1);
    c.normal  = Vector3::nan();
    c.tangent = Vector4::nan();

	CPUVertexArray::Vertex& d = vertexArray.next();
	d.position = (center - u * 0.5f - v * 0.5f) * voxelRes;
    //d.texCoord0=Point2(getTexCoord(d.position,axis));
    d.texCoord0=Point2(0,0);
    d.normal  = Vector3::nan();
    d.tangent = Vector4::nan();

	// If positive, add counterclockwise
	if (sign > 0.0f) {
		mesh->cpuIndexArray.append(index, index + 1, index + 2);
		mesh->cpuIndexArray.append(index, index + 2, index + 3);
    }
	// If negative, add clockwise
	else {
		mesh->cpuIndexArray.append(index, index + 3, index + 2);
		mesh->cpuIndexArray.append(index, index + 2, index + 1);
    }

	// Automatically compute normals. get rid of this when figure out how to modify gpuNormalArray   ????
	ArticulatedModel::CleanGeometrySettings geometrySettings;
    geometrySettings.allowVertexMerging = false;
    m_model->cleanGeometry(geometrySettings);

	// If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
	mesh->clearIndexStream();

}

void App::removeVoxel(Point3int32 input) {

}



shared_ptr<Model> App::initializeModel() {
    ArticulatedModel::Part*     part      = m_model->addPart("root");

	// Tell m_model to generate bounding boxes, GPU vertex arrays, normals, and tangents automatically
	ArticulatedModel::CleanGeometrySettings geometrySettings;
    geometrySettings.allowVertexMerging = false;
    m_model->cleanGeometry(geometrySettings);

	return m_model;
}

void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt){
     GApp::onSimulation(rdt, sdt, idt);


     //if(m_firstPersonMode){
     //   CFrame c = player.position;
     //   c.translation += Vector3(0, 0.6f, 0); // Get up to head height
     //   c.rotation = c.rotation * Matrix3::fromAxisAngle(Vector3::unitX(), player.headTilt);
     //   activeCamera()->setFrame(c);
     //
     //   movePlayer(sdt);
     //}


    


    // Example GUI dynamic layout code.  Resize the debugWindow to fill
    // the screen horizontally.
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


void App::movePlayer(SimTime deltaTime){
    Vector3 velocity = player.position.vectorToWorldSpace(player.desiredOS);
    player.position.translation += velocity * (float)deltaTime;

    player.heading += player.desiredYaw * (float)deltaTime;
    player.position.rotation     = Matrix3::fromAxisAngle(Vector3::unitY(), player.heading);

    player.headTilt = clamp(player.headTilt + player.desiredPitch, -80 * units::degrees(), 80 * units::degrees());
}

Point3int32 App::cameraIntersectVoxel(){
    Point2 center = UserInput(this->window()).mouseXY();
    Ray cameraRay = activeCamera()->worldRay(center.x, center.y, renderDevice->viewport());
    
    int maxSteps = 100;
    float stepDistance = (0.1f * voxelRes);
    bool intersect = false;
    Point3 lastOpen = (cameraRay.origin() / voxelRes);
    Point3 testPos = (cameraRay.origin());
    Vector3 direction = cameraRay.direction();

    for(int i = 0; i < maxSteps && !intersect; ++i){
       testPos = testPos + direction*stepDistance;
       Point3 voxelTest = testPos / voxelRes;
       if(m_posToVox.containsKey( (Point3int32)(voxelTest)  )){
            intersect = true;
       }else{
        lastOpen = voxelTest;
       }
    }

    if(intersect && !m_posToVox.containsKey((Point3int32)lastOpen)){
        return (Point3int32)lastOpen;
    }else{
        Point3int32 nothing;
        return nothing;
    }




}

bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }


    if((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey(' '))){
        Point3int32 voxelHit = cameraIntersectVoxel();
        Point3int32 nothing;
        if( voxelHit != nothing){
            addVoxel( voxelHit, 0);
        }
        }



    //if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) {
    //    m_firstPersonMode = ! m_firstPersonMode;
    //    const shared_ptr<Camera>& camera = m_firstPersonMode ? scene()->defaultCamera() : debugCamera();
    //    setActiveCamera(camera);
    //}
    //
    //if (event.key.keysym.sym == GKey('w')){
    //    if((event.type == GEventType::KEY_DOWN)){
    //        player.desiredOS = Vector3(player.speed, 0, 0);
    //    }else{
    //        player.desiredOS = Vector3(0,0,0);
    //    }
    //}

    return false;
}

void App::onUserInput(UserInput* ui) {
    
    super::onUserInput(ui);
    //ui->setPureDeltaMouse(m_firstPersonMode);
    //
    //if(m_firstPersonMode){
    //    const float   pixelsPerRevolution = 30;
    //    const float   turnRatePerPixel  = -pixelsPerRevolution * units::degrees() / (units::seconds()); 
    //    const float   tiltRatePerPixel  = -0.2f * units::degrees() / (units::seconds()); 
    //
    //    const Vector3& forward = -Vector3::unitZ();
    //    const Vector3& right   =  Vector3::unitX();
    //
    //    Vector3 linear  = Vector3::zero();
    //    float   yaw = 0.0f;
    //    float   pitch = 0.0f;
    //    linear += forward * ui->getY() * player.speed;
    //    linear += right   * ui->getX() * player.speed;
    //    
    //    yaw     = ui->mouseDX() * turnRatePerPixel;
    //    pitch   = ui->mouseDY() * tiltRatePerPixel;
    //
    //    static const Vector3 jumpVelocity(0, 40 * units::meters() / units::seconds(), 0);
    //
    //
    //    linear += player.desiredOS;
    //    
    //    
    //    player.desiredOS= linear;
    //    player.desiredYaw = yaw;
    //    player.desiredPitch = pitch;
    //}
    
}

void App::onGraphics(RenderDevice * rd, Array< shared_ptr< Surface > > & surface, Array< shared_ptr< Surface2D > > & surface2D ) {
    super::onGraphics(rd, surface, surface2D);

}
