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

    // NON-VR CODE //////////////////////
    GApp::Settings settings(argc, argv);
    //////////////////////////////
    
    // VR CODE////////////////


   // VR CODE////////////////
   //VRApp::Settings settings(argc, argv);
   //settings.vr.debugMirrorMode = //VRApp::DebugMirrorMode::NONE;//
   //    VRApp::DebugMirrorMode::PRE_DISTORTION;
   //
   //settings.vr.disablePostEffectsIfTooSlow = true;
   //    settings.renderer.deferredShading   = true;
   // settings.renderer.orderIndependentTransparency = true;
   ///////////////////////////


    settings.window.caption             = argv[0];
    settings.window.width               = 1280; settings.window.height       = 720; settings.window.fullScreen          = false;
    settings.window.resizable           = ! settings.window.fullScreen;
    settings.window.framed              = ! settings.window.fullScreen;
    settings.window.asynchronous        = true; //false if no vr
    
    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(0, 0); // 64 64 for non vr
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir                    = FileSystem::currentDirectory();
    settings.screenshotDirectory        = "../journal/";
    
    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = true; // false if no vr

    return App(settings).run();
}



App::App(const AppBase::Settings& settings) : super(settings) {
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
        containerPane->addNumberBox("Voxel Type", &m_voxelType,"",GuiTheme::LINEAR_SLIDER, 0,1,1);

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

    initializeMenu();

    //makeFP();

    updateSelect();

}

void App::updateSelect(){
    Ray cameraRay;
    Ray empty;

    if(menu){
        if(vrEnabled){
            //if(m_vrControllerArray.size() > select.menuControllerIndex){
            //    cameraRay = m_vrControllerArray[select.menuControllerIndex]->frame().lookRay();
            //    cameraRay = Ray(cameraRay.origin() + m_controllerOffset, cameraRay.direction());
            //}
            //
            //select.buttonSelected = false;
            //for(int i = 0; i < menuButtons.length() && !select.buttonSelected; ++i){
            //    if(( cameraRay.origin() - menuFrame.pointToWorldSpace(menuButtons[i]) ).length() <= 0.2){
            //        select.buttonSelected = true;
            //        select.buttonIndex = i;
            //    }    
            //}
            //
            //if(cameraRay.origin() != empty.origin()){
            //    select.lookDirection = cameraRay.direction();
            //    select.position = cameraRay.origin() + 1*select.lookDirection.direction();
            //    select.ray = cameraRay;
            //    drawSelection();
            //}
        }
    
    }else{
        if(vrEnabled){
           // if(m_vrControllerArray.size() > 0){
           //     cameraRay = m_vrControllerArray[0]->frame().lookRay();
           //     cameraRay = Ray(cameraRay.origin() + m_controllerOffset, cameraRay.direction());
           // }
           //
            
        }else{
            Point2 center = Point2(UserInput(this->window()).mouseXY().x / this->window()->width(), UserInput(this->window()).mouseXY().y / this->window()->height());
            cameraRay = activeCamera()->worldRay(center.x * renderDevice->viewport().width(), center.y * renderDevice->viewport().height(), renderDevice->viewport());
           
        }

        if(cameraRay.origin() != empty.origin()){
            select.lookDirection = cameraRay.direction();
            select.position = cameraRay.origin() + 1*select.lookDirection.direction();
            select.ray = cameraRay;
            drawSelection();
        }
    
    }


    


}

void App::drawSelection(){



    if(menu){
        if(select.buttonSelected){
            debugDraw(Sphere(menuFrame.pointToWorldSpace(menuButtons[select.buttonIndex]), 0.3), 0.0f, Color3::white());
        }
    }else{

        Point3int32 lastOpen;
        Point3int32 voxelTest;
        cameraIntersectVoxel(lastOpen, voxelTest);
        Point3 voxelHit = voxelToWorldSpace(voxelTest);
        Point3 sideHit = voxelToWorldSpace(lastOpen);
        Vector3 side = sideHit - voxelHit;
        sideHit = voxelHit + side*0.2;
        //debugDraw(Sphere(voxelHit, 0.3));
        //debugDraw(Sphere(sideHit, 0.2), 0.0f, Color3::blue());

        debugDraw( Box(voxelHit - Point3(voxelRes/2,voxelRes/2,voxelRes/2),voxelHit + Point3(voxelRes/2,voxelRes/2,voxelRes/2)) );
        if (lastOpen != voxelTest) {
            debugDraw( Box(sideHit - Point3(voxelRes/3,voxelRes/3,voxelRes/3),sideHit + Point3(voxelRes/3,voxelRes/3,voxelRes/3)), 0.0f, Color3::blue() );
        }
    }
}

void App::initializeScene() {
    m_posToChunk = Table<Point2int32, shared_ptr<Table<Point3int32, int>>>();

    m_voxToProp = Table<int, Any>();

    m_chunksToRedraw = Array<Point2int32>();
	
    for (int i = 0; i < voxTypeCount; ++i) {
        m_voxToProp.set(i, Any::fromFile(format("data-files/voxelTypes/vox%d.Any", i)));
    }

	initializeMaterials();

    addVoxelModelToScene();

    // Initialize ground
    for(int x = -25; x < 25; ++x) {
        for(int z = -25; z < 25; ++z) {
            setVoxel(Point3int32(x,0,z), 0);
        }
    }
    redrawWorld();
}


void App::initializeMenu(){
    menuButtons.append(Point3(-0.5,0,-0.5));
    buttonColors.append(Color3::green());


    menuButtons.append(Point3(0.5, 0, -0.5));
    buttonColors.append(Color3::brown());

}


void App::initializeMaterials() {
	m_voxToMat = Table<int, shared_ptr<UniversalMaterial>>();

	// using morgan's premade materials. is there a way to access them without moving them to the local data-files directory?
//	for (int i = 0; i < voxTypeCount; ++i) {
//		Any any = m_voxToProp.get(i);
//		String materialName = any["material"];
//		m_voxToMat.set(i, UniversalMaterial::create( Any::fromFile(format("data-files/texture/%s/%s.UniversalMaterial.Any", materialName, materialName)) ));
//	}

	// for now just hard code each individual material?
	m_voxToMat.set(0, UniversalMaterial::create( Any::fromFile("data-files/texture/greengrass/greengrass.UniversalMaterial.Any") ));
	m_voxToMat.set(1, UniversalMaterial::create( Any::fromFile("data-files/texture/rockwall/rockwall.UniversalMaterial.Any") ));

}

void App::initializeModel() {
    ArticulatedModel::Part* part = m_model->addPart("root");

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
}

//Returns true if the positions given is occupied.
bool App::voxIsSet(Point3int32 pos) {
    return getChunk(pos)->containsKey(pos);
}


// Conversions between voxelspace and worldspace, the hard coded point3 is because voxels are drawn in center 
Point3 App::voxelToWorldSpace(Point3int32 voxelPos) {
    return Point3(voxelPos) * voxelRes + Point3(0.5, 0.5f, 0.5f);
}

//Ditto
Point3int32 App::worldToVoxelSpace(Point3 worldPos){
    return (Point3int32)((worldPos - m_sceneOffset - Point3(0.5, 0.5f, 0.5f)) / voxelRes);
}

//Returns the chunk coords for a given point.
//I changed this I hope it works -Youle
Point2int32 App::getChunkCoords(Point3int32 pos) {
	// Chunks between -7 and 7 are too big, modify
    int32 addX = 0;
    int32 addZ = 0;
    if (pos.x < 0) {
        addX = -1;
    }
    if (pos.z < 0) {
        addZ = -1;
    }
    return Point2int32((pos.x / chunkSize) + addX, (pos.z / chunkSize) + addZ);
}



//Used explicitly for removing voxels, check whether the voxel is the boundary of a chunk, if it is, 
//push the neighboring chunk to the m_chunksToRedraw for later update. 
//Can add up to 2 more chunks for update
void App::checkBoundaryAdd(Point3int32 pos){
    Point2int32 current = getChunkCoords(pos);
    Point2int32 xPlus   = getChunkCoords(pos + Point3int32(1,0,0));
    Point2int32 xMinus  = getChunkCoords(pos + Point3int32(-1,0,0));
    Point2int32 zPlus   = getChunkCoords(pos + Point3int32(0,0,1));
    Point2int32 zMinus  = getChunkCoords(pos + Point3int32(0,0,-1));

    if ( (xPlus != current) && (!m_chunksToRedraw.contains(xPlus)) ) {
        m_chunksToRedraw.push(xPlus);
    } 
    if ( (xMinus != current) && (!m_chunksToRedraw.contains(xMinus)) ) {
        m_chunksToRedraw.push(xMinus);
    }
    if ( (zPlus != current) && (!m_chunksToRedraw.contains(zPlus)) ) {
        m_chunksToRedraw.push(zPlus);
    } 
    if ( (zMinus != current) && (!m_chunksToRedraw.contains(zMinus)) ) {
        m_chunksToRedraw.push(zMinus);
    }
}


//Return the chunk a given voxel resides in.
shared_ptr<Table<Point3int32, int>> App::getChunk(Point3int32 pos) {
    Point2int32 key = getChunkCoords(pos);
    if (!m_posToChunk.containsKey(key)) {
        m_posToChunk.set( key, std::make_shared<Table<Point3int32, int>>(Table<Point3int32, int>()) );
    }
    return m_posToChunk[key];
}

//Return the voxel type at a given grid position.
int App::posToVox(Point3int32 pos) {
    return getChunk(pos)->operator[](pos);
}

//Set the voxel at a given grid position in the world data structure.
void App::setVoxel(Point3int32 pos, int type) {
    shared_ptr<Table<Point3int32, int>> chunk = getChunk(pos);
	if ( !voxIsSet(pos) ) {
		chunk->set(pos, type);
	}
}

// Unset the voxel at a given grid position in the world data structure.
void App::unsetVoxel(Point3int32 pos) {
    shared_ptr<Table<Point3int32, int>> chunk = getChunk(pos);
	if ( voxIsSet(pos) ) {
		chunk->remove(pos);
	}
}

// Redraw the geometry for a given chunk.
void App::redrawChunk(Point2int32 chunkPos) {
	// Clear CPU vertex and CPU index arrays for every chunk type
    for (int i = 0; i < voxTypeCount; i++) {
	    if ( notNull(m_model->geometry(format("geom %d,%d,%d", chunkPos.x, chunkPos.y, i))) ) {
        ArticulatedModel::Geometry* geometry = m_model->geometry(format("geom %d,%d,%d", chunkPos.x, chunkPos.y, i ));
        ArticulatedModel::Mesh*     mesh     = m_model->mesh(format("mesh %d,%d,%d", chunkPos.x, chunkPos.y, i ));

	    Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	    Array<int>&					   indexArray  = mesh->cpuIndexArray;

	    vertexArray.fastClear();
	    indexArray.fastClear();
        }
    }

	Table<Point3int32, int>::Iterator it = m_posToChunk[chunkPos]->begin();
	for (it; it != m_posToChunk[chunkPos]->end(); ++it) {
		drawVoxel( (*it).key );
	}

	// I replaced this with above iterator. hope it works - Lylia
//   Array<Point3int32> voxArray = m_posToChunk[chunkPos]->getKeys();
//   for (int i = 0; i < voxArray.size(); ++i) {
//       drawVoxel( voxArray[i] );
//   }

    int index = m_chunksToRedraw.findIndex(chunkPos);
    if (index > -1) {
        m_chunksToRedraw.remove(index);
    }
}

//Redraw the geometry for the chunks that need to be updated.
//SCREW RUNCONCURRENTLY AMIRIGHT(sorry Ben)

void App::redrawChunks() {
    /*Thread::runConcurrently(0, m_chunksToRedraw.size(), [&](int i) {
        redrawChunk(m_chunksToRedraw[i]);});*/
    for (int i = 0; i < m_chunksToRedraw.size(); ++i) {
        redrawChunk(m_chunksToRedraw[i]);
    }
}

//Redraw the geometry for the entire voxel world.
void App::redrawWorld() {
    Array<Point2int32> chunkArray = m_posToChunk.getKeys();
    for (int i = 0; i < chunkArray.size(); ++i) {
        redrawChunk(chunkArray[i]);
    }
}

 

void App::drawVoxel(Point3int32 input) {
    int type = posToVox(input);

	// Check each position adjacent to voxel, and if nothing is there, add a face
    if ( !voxIsSet(input + Vector3int32(1,0,0)) ) {
        addFace(input, Vector3int32(1,0,0), Vector3::X_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(-1,0,0)) ) {
        addFace(input, Vector3int32(-1,0,0), Vector3::X_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(0,1,0)) ) {
        addFace(input, Vector3int32(0,1,0), Vector3::Y_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(0,-1,0)) ) {
        addFace(input, Vector3int32(0,-1,0), Vector3::Y_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(0,0,1)) ) {
        addFace(input, Vector3int32(0,0,1), Vector3::Z_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(0,0,-1)) ) {
        addFace(input, Vector3int32(0,0,-1), Vector3::Z_AXIS, type);
    }
}

// Input = Center of vox
void App::addVoxel(Point3int32 input, int type) {
    setVoxel(input, type);
    drawVoxel(input);
}

void App::addFace(Point3int32 input, Vector3 normal, Vector3::Axis axis, int type) {
    Point2int32 chunkCoords = getChunkCoords(input);
    if ( isNull(m_model->geometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type))) ) {
		ArticulatedModel::Geometry* geometry = m_model->addGeometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type));
		ArticulatedModel::Mesh*		mesh	 = m_model->addMesh(format("mesh %d,%d,%d", chunkCoords.x, chunkCoords.y, type), m_model->part("root"), geometry);
		mesh->material = m_voxToMat[type];
	}
    ArticulatedModel::Geometry* geometry = m_model->geometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type));
    ArticulatedModel::Mesh*     mesh	 = m_model->mesh(format("mesh %d,%d,%d", chunkCoords.x, chunkCoords.y, type));

	// Center of face we are adding
	Point3 center = voxelToWorldSpace(input) + normal * (0.5f * voxelRes);

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
	a.position = center + ((u * 0.5f - v * 0.5f) * voxelRes);
    a.texCoord0=Point2(1, 0);
    a.normal  = Vector3::nan();
    a.tangent = Vector4::nan();

	CPUVertexArray::Vertex& b = vertexArray.next();
	b.position = center + ((u * 0.5f + v * 0.5f) * voxelRes);
    b.texCoord0 = Point2(1, 1);
    b.normal  = Vector3::nan();
    b.tangent = Vector4::nan();

	CPUVertexArray::Vertex& c = vertexArray.next();
	c.position = center + ((-u * 0.5f + v * 0.5f) * voxelRes);
    c.texCoord0 = Point2(0, 1);
    c.normal  = Vector3::nan();
    c.tangent = Vector4::nan();

	CPUVertexArray::Vertex& d = vertexArray.next();
	d.position = center + (( -u * 0.5f - v * 0.5f) * voxelRes);
    d.texCoord0=Point2(0, 0);
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
	unsetVoxel(input);

	Point2int32 chunkCoords = getChunkCoords(input);
    if( !m_chunksToRedraw.contains(chunkCoords) ) {
        m_chunksToRedraw.push(chunkCoords);
    }

    checkBoundaryAdd(input);
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





void App::cameraIntersectVoxel(Point3int32& lastPos, Point3int32& hitPos){ //make this work
   
    const float maxDist = 10.0f;
    Vector3 direction = select.lookDirection;
    
    Ray cameraRay(select.position, select.lookDirection);

    //Point2 center = UserInput(this->window()).mouseXY();
    //Ray cameraRay = activeCamera()->worldRay(center.x / this->window()->width() * renderDevice->width(), center.y / this->window()->height() * renderDevice->height(), renderDevice->viewport());

    hitPos = Point3int32( (select.position + select.lookDirection * maxDist) / voxelRes );

    lastPos = hitPos;

    //the Boundary of the voxels that would intersect
    Point3int32 voxelBound = Point3int32(1<<15, 1<<15, 1<<15);
    
    for (RayGridIterator it(cameraRay, voxelBound, Vector3(voxelRes,voxelRes,voxelRes), Point3(-voxelBound / 2) * voxelRes, -voxelBound / 2); it.insideGrid(); ++it) {
    // Search for an intersection within this grid cell
        if( voxIsSet(it.index()) ) {
            hitPos = it.index();
            lastPos = it.index() + it.enterNormal();
            break;
        }
    }
    
}

bool App::onEvent(const GEvent& event) {



    //if((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey(' '))){
    if (event.isMouseEvent() && event.button.type == GEventType::MOUSE_BUTTON_CLICK) {
          // Left mouse
          if (event.button.button == (uint8)0) {
            Point3int32 hitPos;
            Point3int32 lastPos;
            cameraIntersectVoxel(lastPos, hitPos);
            addVoxel(lastPos, m_voxelType);
          
          // Middle mouse
          } else if (event.button.button == (uint8)1) {
            Point3int32 hitPos;
            Point3int32 lastPos;
            cameraIntersectVoxel(lastPos, hitPos);
            removeVoxel(hitPos);
          }
    }
	else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('l')) ){ 
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
		m_currentMark = hitPos;
    }
	else if ( (event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('l')) ){
		// determine how big the radius is
		Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        selectCircle(m_currentMark, Vector3(hitPos.x - m_currentMark.x, hitPos.y - m_currentMark.y, hitPos.z - m_currentMark.z).magnitude());
	}
	else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('u')) ) {
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        m_currentMark = hitPos;
        //Change mode to Elevating.
    }
	else if ( (event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('u')) ) {
        //Also check mode to see if it is Elevating.
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

    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }

    return false;
}

void App::selectCircle(Point3int32 center, int radius) {
    m_selection.clear();
    
	// Ray from camera to center
	// Ray from camera to radius endpoint

    for (int y = center.y - radius; y <= center.y + radius; ++y) {
        for (int x = center.x - radius; x <= center.x + radius; ++x) {
            for (int z = center.z-radius; z <= center.z + radius; ++z) {
                Point3int32 pos = Point3int32(x,y,z);

				// check if the voxel is in the sphere
                if(sqrt((x-center.x) * (x-center.x) + (y-center.y) * (y-center.y) + (z-center.z) * (z-center.z)) <= radius && voxIsSet(pos)) {
                    m_selection.append(Point3int32(x,y,z));
                }
            }
        }
    }
}

void App::elevateSelection(int delta) {
    if (delta != 0) {
        for (int i = 0; i < m_selection.size(); ++i) {
            Point3int32 targetPos = m_selection[i] + Point3int32(0, delta, 0);
            setVoxel(targetPos, posToVox(m_selection[i]));
            Point2int32 chunkCoords = getChunkCoords(m_selection[i]);
            if( !m_chunksToRedraw.contains(chunkCoords) ) {
                m_chunksToRedraw.push(chunkCoords);
            }
            unsetVoxel(m_selection[i]);
        }
    }
}

void App::onUserInput(UserInput* ui) {
    
    super::onUserInput(ui);

    if(!vrEnabled){
        updateSelect();
    }



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

    redrawChunks();

    if(vrEnabled){
        //updateSelect();
        //Point3 head;
        //Point3 hand1;
        //Point3 hand2;
        //if(m_vrControllerArray.size() > 0){
        //    hand1 = m_vrControllerArray[0]->frame().pointToWorldSpace(Point3(0,0,0));
        //    hand1 += m_controllerOffset;
        //    debugDraw(Sphere(hand1, 0.1), 0.0f, Color3::blue());
        //}
        //if(m_vrControllerArray.size() > 1){
        //    hand2 = m_vrControllerArray[1]->frame().pointToWorldSpace(Point3(0,0,0));
        //    hand2 += m_controllerOffset;
        //    debugDraw(Sphere(hand2, 0.1), 0.0f, Color3::orange());
        //}
        //
        //
        //
        //if(menu){
        //   CFrame frame = menuFrame;
        //   for(int i = 0; i < menuButtons.length(); ++i){
        //    debugDraw(Sphere(menuFrame.pointToWorldSpace(menuButtons[i]), 0.2), 0.0f, buttonColors[i]);
        //   }
        //}
        //


        //VR CONTROLLER INPUT
        //BEGIN_PROFILER_EVENT("VRApp::sampleTrackingData");

        //vr::VREvent_t vrEvent;
        //while (m_hmd->PollNextEvent(&vrEvent, sizeof(vrEvent))) {
        //    debugPrintf("Device %d sent button %d press\n", vrEvent.trackedDeviceIndex, vrEvent.data.controller.button);
        //    switch (vrEvent.eventType) {
        //    // Device 0 is the HMD (probably). Device 1 and 2 are the controllers (probably) on Vive.
        //    // Button 32 = k_EButton_Axis0 is the large DPad/touch button on the Vive.
        //    case vr::VREvent_ButtonPress:
        //        if(!menu){
        //            Point3int32 lastPos;
        //            Point3int32 hitPos;
        //            switch(vrEvent.data.controller.button){
        //            
        //            //trigger
        //            case 33:
        //               
        //                cameraIntersectVoxel(lastPos, hitPos);
        //                debugPrintf("AddVoxel %d %d %d\n", lastPos.x, lastPos.y, lastPos.z);
        //
        //                addVoxel(lastPos, m_voxelType);
        //                break;
        //            
        //            //Grip
        //            case 2:
        //              
        //                cameraIntersectVoxel(lastPos, hitPos);
        //                removeVoxel(hitPos);
        //                break;
        //            
        //            //Touchpad Click
        //            case 32:
        //              
        //                cameraIntersectVoxel(lastPos, hitPos);
        //                m_debugCamera->setFrame(CFrame(voxelToWorldSpace(lastPos)));
        //                m_debugController->setFrame(m_debugCamera->frame());
        //                updateSelect();
        //                
        //                break;
        //            
        //            default:
        //                debugPrintf("INPUT NOT RECOGNIZED\n");
        //                break;
        //            }
        //           
        //  
        //        }else{
        //            switch(vrEvent.data.controller.button){
        //            
        //            //trigger
        //            case 33:
        //                if(select.buttonSelected){
        //                    m_voxelType = select.buttonIndex;
        //                
        //                }
        //                break;
        //           
        //            
        //            default:
        //                debugPrintf("INPUT NOT RECOGNIZED\n");
        //                break;
        //            }
        //        }
        //
        //       if(vrEvent.data.controller.button == 1){
        //            menu = !menu;
        //            debugPrintf("MENU %d\n", menu);
        //            if(menu){
        //                menuFrame = m_vrHead->frame();
        //                select.menuControllerIndex = (vrEvent.trackedDeviceIndex - 1) % 2;
        //            }
        //       }
        //
        //       break;
        //
        //    case vr::VREvent_ButtonUnpress:
        //        debugPrintf("Device %d sent button %d unpress\n", vrEvent.trackedDeviceIndex, vrEvent.data.controller.button);
        //        break;
        //
        //    default:
        //        // Ignore event
        //        ;
        //    }
        //}

        //END_PROFILER_EVENT();
        
        
        


    }

    super::onGraphics(rd, surface, surface2D);


}
