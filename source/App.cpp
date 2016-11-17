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
			addVoxel(Point3int32(0,0,-5), 1);
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

    loadScene("G3D Whiteroom");

	initializeScene();
}


void App::initializeScene() {
    m_posToVox = Table<Point3int32, int>();

    m_voxToProp = Table<int, Any>();

	
    for(int i = 0; i < voxTypeCount; ++i) {
        m_voxToProp.set(i, Any::fromFile(format("voxelTypes/vox%d.Any", i)));
    }

	initializeMaterials();

    addVoxelModelToScene();

    initializePlayer();
    // Initialize ground
    for(int x = -5; x < 5; ++x) {
        for(int z = -5; z < 5; ++z) {
            addVoxel(Point3int32(x,0,z), 0);
        }
    }
}

void App::initializePlayer(){
}

void App::initializeMaterials() {
	m_voxToMat = Table<int, shared_ptr<UniversalMaterial>>();

	for (int i = 0; i < voxTypeCount; ++i) {
		Any any = m_voxToProp.get(i);

		// ???????????????????????????????????????????????????????????????????
//		shared_ptr<UniversalMaterial> mat = UniversalMaterial::create(
//			PARSE_ANY(
//				UniversalMaterial::Specification {
//				lambertian = Texture::Specification {
//					filename = "data-files/texture/grass";
//				};
//				}
//			)
//			//specification
//		);
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

void App::initializeModel() {
    ArticulatedModel::Part*		part	 = m_model->addPart("root");

}

// Create a cube model. Code pulled from sample/proceduralGeometry
void App::addVoxelModelToScene() {
    // Replace any existing voxel model. Models don't 
    // have to be added to the model table to use them 
    // with a VisibleEntity.
    initializeModel();
    if (scene()->modelTable().containsKey(m_model->name())) {
        scene()->removeModel(m_model->name());
    }
    scene()->insert(m_model);

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
	if ( !m_posToVox.containsKey(input) ) {
		m_posToVox.set(input, type);
	}

	if ( isNull(m_model->geometry("geom")) ) {
		ArticulatedModel::Geometry* geometry = m_model->addGeometry("geom");
		ArticulatedModel::Mesh*		mesh	 = m_model->addMesh("mesh", m_model->part("root"), geometry);
		mesh->material = m_voxToMat[0];
	}

	// Check each position adjacent to voxel, and if nothing is there, add a face
    if ( !m_posToVox.containsKey(input + Vector3int32(1,0,0)) ) {
        addFace(input, Vector3int32(1,0,0), Vector3::X_AXIS, type);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(-1,0,0)) ) {
        addFace(input, Vector3int32(-1,0,0), Vector3::X_AXIS, type);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,1,0)) ) {
        addFace(input, Vector3int32(0,1,0), Vector3::Y_AXIS, type);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,-1,0)) ) {
        addFace(input, Vector3int32(0,-1,0), Vector3::Y_AXIS, type);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,0,1)) ) {
        addFace(input, Vector3int32(0,0,1), Vector3::Z_AXIS, type);
    }
    if ( !m_posToVox.containsKey(input + Vector3int32(0,0,-1)) ) {
        addFace(input, Vector3int32(0,0,-1), Vector3::Z_AXIS, type);
    }

}

void App::addFace(Point3int32 input, Vector3 normal, Vector3::Axis axis, int type) {
    ArticulatedModel::Geometry* geometry = m_model->geometry("geom");
    ArticulatedModel::Mesh*     mesh	 = m_model->mesh("mesh");

	// Center of face we are adding
	Point3 center = Point3(input) + normal * 0.5f;

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

	// If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
	geometry->clearAttributeArrays();

	// If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
	mesh->clearIndexStream();

}

void App::removeVoxel(Point3int32 input) {
	m_posToVox.remove(input);

	ArticulatedModel::Geometry* geometry = m_model->geometry("geom");
    ArticulatedModel::Mesh*     mesh = m_model->mesh("mesh");

	// Remake the entire CPU vertex and CPU index arrays
	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>&					   indexArray  = mesh->cpuIndexArray;

	vertexArray.fastClear();
	indexArray.fastClear();

	Array<Point3int32> voxArray = m_posToVox.getKeys();
	for (int i = 0; i < voxArray.size(); ++i) {
		addVoxel( voxArray[i], m_posToVox.get(voxArray[i]) );
	}
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

//helper function for cameraIntersectVoxel
float App::maxDistGrid(Point3 pos, Vector3 dir){
    pos.x;
    dir.x;
    float a;
    float b;
    float c;
    float temp;
    
    temp=pos.x/voxelRes;

    if(dir.x>=0){
        a=ceil(temp)-temp; 
    } else {
        a=floor(temp)-temp;
    }
    
    temp=pos.y/voxelRes;

    if(dir.y>=0){
        
        b=ceil(temp)-temp; 
    } else {
        b=floor(temp)-temp;
    }
    
    temp=pos.z/voxelRes;

    if(dir.z>=0){
        
        c=ceil(temp)-temp; 
    } else {
        c=floor(temp)-temp;
    }
    a = abs(a);
    b = abs(b);
    c = abs(c);
    return max(a,b,c)*voxelRes;
    

}



void App::cameraIntersectVoxel(Point3int32& lastOpen, Point3int32& voxelTest){ //make this work
    Point2 center = UserInput(this->window()).mouseXY();
    Ray cameraRay = activeCamera()->worldRay(center.x / this->window()->width() * renderDevice->width(), center.y / this->window()->height() * renderDevice->height(), renderDevice->viewport());
    
    const float shortDist = 1e-1;
    const int maxSteps = 20;
    const float maxDist = 10.0f;
    float totalDist = 0.0f;
    bool intersect = false;
    lastOpen = Point3int32(cameraRay.origin() / voxelRes);
    Point3 testPos = (cameraRay.origin());
    Vector3 direction = cameraRay.direction();
    

    for(int i = 0; i < maxSteps && !intersect; ++i){
        if(totalDist > maxDist) {
            break;
        }
       float stepDistance = max(shortDist,maxDistGrid(testPos,direction));
       totalDist += stepDistance;
       testPos = testPos + direction*stepDistance;
       voxelTest = Point3int32(testPos / voxelRes + (voxelRes * Point3(0.5f,0.5f,0.5f)));
       if(m_posToVox.containsKey( voxelTest  )){
            intersect = true;
       }else if(lastOpen!=voxelTest){
            lastOpen = voxelTest;
            i = 0;
       }
    }
}

bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }


    if((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey(' '))){
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        addVoxel( lastPos, 0);
   
    } else if((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('r'))){ 
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        removeVoxel( hitPos);
    } else if((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('l'))){ 
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        selectCircle( hitPos, 5);
    } else if((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('u'))) {
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        m_currentMark = hitPos;
        //Change mode to Elevating.
    } else if((event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('u'))) {
        //Also check mode to see if it is Eleveating.
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        elevateSelection(hitPos.y - m_currentMark.y);
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

void App::selectCircle(Point3int32 center, int radius) {
    m_selection.clear();
    
    for (int y = center.y - 5; y <= center.y + 5; ++y) {
        for (int x = center.x - radius; x <= center.x + radius; ++x) {
            for (int z = center.z-radius; z <= center.z + radius; ++z) {
                Point3int32 pos = Point3int32(x,y,z);
                if(sqrt((x- center.x) * (x-center.x) + (z-center.z) * (z-center.z)) <= radius && m_posToVox.containsKey(pos)) {
                    m_selection.append(Point3int32(x,y,z));
                }
            }
        }
    }
}

void App::elevateSelection(int delta) {
    for (int i = 0; i < m_selection.size(); ++i) {
        addVoxel(m_selection[i] + Point3int32(0, delta, 0), m_posToVox[m_selection[i]]);
    }
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
