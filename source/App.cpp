/** \file App.cpp */
#include "App.h"
#include "Selection.h"
#include "Util.h"
#define PI 3.1415926f

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = true;
        initGLG3D(g3dSpec);
    }

    // NON-VR CODE //////////////////////
    GApp::Settings settings(argc, argv);
    //////////////////////////////
    
    // VR CODE////////////////


   // VR CODE////////////////
   //VRApp::Settings settings(argc, argv);
   //settings.vr.debugMirrorMode = //VRApp::DebugMirrorMode::NONE;//
   //VRApp::DebugMirrorMode::PRE_DISTORTION;
   //
   // settings.vr.disablePostEffectsIfTooSlow = true;
   // settings.renderer.deferredShading   = true;
   // settings.renderer.orderIndependentTransparency = true;
   ///////////////////////////


    settings.window.caption             = argv[0];
    settings.window.width               = 1280; settings.window.height       = 720; settings.window.fullScreen          = false;
    settings.window.resizable           = ! settings.window.fullScreen;
    settings.window.framed              = ! settings.window.fullScreen;
    settings.window.asynchronous        = true; // false if no vr
    
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

		containerPane->addLabel("Left click: Add voxel");
		containerPane->addLabel("Middle click: Remove voxel");
		containerPane->addLabel("j: Box select");
		containerPane->addLabel("k: Cylinder select");
		containerPane->addLabel("l: Sphere select");
		containerPane->addLabel("u: Elevate selection");
		containerPane->addLabel("r: Change intersection distance");
		containerPane->addLabel("f: Toggle force intersect");

		m_typesList.append("Grass", "Rock", "Brick", "Sand", "Rough Cedar", "Rusty Metal");
		m_typesList.append("Chrome", "Black Rubber");
        containerPane->addDropDownList("Voxel Type", m_typesList, &m_voxelType);

		containerPane->pack();
	}

}

// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be
// automatically caught.
void App::onInit() {
    super::onInit();
    setFrameDuration(1.0f / 120.0f);

    showRenderingStats = false;

    makeGUI();
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));

    loadScene("Empty Scene");

	initializeScene();

    updateSelect();

}

void App::updateSelect(){
    Ray cameraRay;
    Ray empty;

    if (menuMode) {
        if (vrEnabled) {
            //if(m_vrControllerArray.size() > m_crosshair.menuControllerIndex){
            //    cameraRay = m_vrControllerArray[ m_crosshair.menuControllerIndex]->frame().lookRay();
            //    cameraRay = Ray(cameraRay.origin(), cameraRay.direction());
            //}
            //
            // m_crosshair.buttonSelected = false;
            //for(int i = 0; i < m_menuButtons.length() && ! m_crosshair.buttonSelected; ++i){
            //    if(( cameraRay.origin() - menuFrame.pointToWorldSpace(m_menuButtons[i]) ).length() <= 0.3){
            //         m_crosshair.buttonSelected = true;
            //         m_crosshair.buttonIndex = i;
            //    }    
            //}
            //
            //if(cameraRay.origin() != empty.origin()){
            //     m_crosshair.lookDirection = cameraRay.direction();
            //     m_crosshair.position = cameraRay.origin();
            //     m_crosshair.ray = cameraRay;
            //    drawCrosshair();
            //}
        }
    
    } else {
        if (vrEnabled) {
            //if(m_vrControllerArray.size() > 0){
            //    cameraRay = m_vrControllerArray[0]->frame().lookRay();
            //    cameraRay = Ray(cameraRay.origin(), cameraRay.direction());
            //}
            //
            //
        } else {
            Point2 center = Point2( UserInput(this->window()).mouseXY().x / this->window()->width(), UserInput(this->window()).mouseXY().y / this->window()->height() );
            cameraRay = activeCamera()->worldRay(center.x * renderDevice->viewport().width(), center.y * renderDevice->viewport().height(), renderDevice->viewport());
           
        }

        if (cameraRay.origin() != empty.origin()) {
            m_crosshair.lookDirection = cameraRay.direction();
            m_crosshair.position = cameraRay.origin();
            m_crosshair.ray = cameraRay;
            drawCrosshair();
        }
    
    }

}

void App::drawCrosshair(){

    if (menuMode) {
        if (m_crosshair.buttonSelected) {
            debugDraw(Sphere(menuFrame.pointToWorldSpace(m_menuButtons[m_crosshair.buttonIndex]), 0.3), 0.0f, Color3::white());
        }
    } else {

        Point3int32 lastOpen;
        Point3int32 voxelTest;
        cameraIntersectVoxel(lastOpen, voxelTest);
        Point3 voxelHit = Util::voxelToWorldSpace(voxelTest);
        Point3 sideHit = Util::voxelToWorldSpace(lastOpen);
        Vector3 side = sideHit - voxelHit;

        sideHit = voxelHit + side * 0.01;

        debugDraw( Box(voxelHit - Point3(voxelRes/2,voxelRes/2,voxelRes/2), voxelHit + Point3(voxelRes/2,voxelRes/2,voxelRes/2)) );
        if (lastOpen != voxelTest) {
            debugDraw( Box(sideHit - Point3(voxelRes/2.01,voxelRes/2.01,voxelRes/2.01),sideHit + Point3(voxelRes/2.01,voxelRes/2.01,voxelRes/2.01)), 0.0f, Color3::blue() );
        }
       
    }
}

void App::initializeScene() {
	
    for (int i = 0; i < voxTypeCount; ++i) {
        m_voxToProp.append(Any::fromFile(format("data-files/voxelTypes/vox%d.Any", i)));
    }

	initializeMaterials();

	initializeSounds();

    initializeModel();

    addEntity(m_model, "voxel");

    // Initialize ground

    for(Point3int32 P(-25, -25, -50); P.x < 25; ++P.x) {
        for(P.z = -25; P.z < 25; ++P.z) {
            for(P.y = -50; P.y < 0; ++P.y) {
                setVoxel(P, GRASS);

            }
        }
    }

    getMenuPositions();
    makeMenuModel();
    m_menu = addEntity(m_menuModel, "menuEntity", false);
   
    redrawWorld();
}


void App::getMenuPositions(){

    int rows = 2;
    int totalVoxels = voxTypeCount;
    int blocksPerRow = totalVoxels / rows;
    float rowSeparation = 0.5;
    float menuRadius = 0.75;
    float menuWidth = 0.75 * PI; //radians
    float x, y, z;
    y = 0.25;
    
    for (int i = 0; i < totalVoxels; ++i) {
    
        int j = i % blocksPerRow;
        float a = (float)j * ( menuWidth / (float)(blocksPerRow) );
        a += PI / 4.0f;

        for (int k = 1; k < rows; ++k) {
            if( j == 0 ){
                y -= rowSeparation;
            }
        }
        x = menuRadius * cos(a);
        z = -1.0f * menuRadius * sin(a);
        
        m_menuButtons.append(Point3(x,y,z));
    }
    m_menuButtons.append(Point3(0.0f,0.25f,-menuRadius));
    
}

void App::makeMenuModel() {
    ArticulatedModel::Part* part = m_menuModel->addPart("root");
    int type;
	for (int t = 0; t < voxTypeCount; ++t) {
        type = t % voxTypeCount;

		ArticulatedModel::Geometry* geometry = m_menuModel->addGeometry(format("geom %d", t));
		ArticulatedModel::Mesh*		mesh	 = m_menuModel->addMesh(format("mesh %d", t), m_menuModel->part("root"), geometry);
		mesh->material = m_voxToMat[type];

		Point3 current = m_menuButtons[t];
		drawVoxelNaive(geometry, mesh, current, 0.25f, t);

        ArticulatedModel::CleanGeometrySettings geometrySettings;
        geometrySettings.allowVertexMerging = false;
        m_menuModel->cleanGeometry(geometrySettings);

	    // If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
	    geometry->clearAttributeArrays();

	    // If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
    	mesh->clearIndexStream();
	}
}

const shared_ptr<ArticulatedModel> App::makeVoxelModel(String modelName, int type, float size) {
     const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty(modelName);

    ArticulatedModel::Part*		part	 = model->addPart("root");
	ArticulatedModel::Geometry* geometry = model->addGeometry("geom");
	ArticulatedModel::Mesh*		mesh	 = model->addMesh("mesh", model->part("root"), geometry);
	mesh->material = m_voxToMat[type];
	
	drawVoxelNaive(geometry, mesh, Point3(0,0,0), size, type);
	
	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	model->cleanGeometry(geometrySettings);
	
	// If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
	geometry->clearAttributeArrays();
	
	// If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
	mesh->clearIndexStream();

    return model;
}

void App::initializeMaterials() {

	m_voxToMat.append(
        UniversalMaterial::create( Any::fromFile(System::findDataFile("greengrass/greengrass.UniversalMaterial.Any")) ),
	    UniversalMaterial::create( Any::fromFile(System::findDataFile("rockwall/rockwall.UniversalMaterial.Any")) ),
	    UniversalMaterial::create( Any::fromFile(System::findDataFile("redbrick/redbrick.UniversalMaterial.Any") )),
	    UniversalMaterial::create( Any::fromFile(System::findDataFile("sand/sand.UniversalMaterial.Any") )),
	    UniversalMaterial::create( Any::fromFile(System::findDataFile("roughcedar/roughcedar.UniversalMaterial.Any")) ),
	    UniversalMaterial::create( Any::fromFile(System::findDataFile("rustymetal/rustymetal.UniversalMaterial.Any") )));
    m_voxToMat.append(
        UniversalMaterial::create( Any::fromFile(System::findDataFile("chrome/chrome.UniversalMaterial.Any") )),
        UniversalMaterial::create( Any::fromFile(System::findDataFile("blackrubber/blackrubber.UniversalMaterial.Any") )));
    
    // using vox8.any, if you do add more materials, increase the number 8. This material is used for debug only, and stays at the end of voxToMat.
    m_voxToMat.append(UniversalMaterial::create( Any::fromFile("data-files/texture/glass/glass.UniversalMaterial.Any")));

}

void App::initializeSounds() {

	m_sounds.append(
		Sound::create("data-files/sounds/ui_wood_back.wav"),				// 0 = add
		Sound::create("data-files/sounds/ui_wood_error.wav"),				// 1 = remove
		Sound::create("data-files/sounds/ui_wood_open.wav"),				// 2 = elevate
		Sound::create("data-files/sounds/ui_casual_pops_back.wav"),			// 3 = open menu
		Sound::create("data-files/sounds/ui_casual_pops_close.wav"),		// 4 = close menu
		Sound::create("data-files/sounds/ui_casual_pops_confirm.wav")		// 5 = select from menu (???)
	);

	m_sounds.append(
		Sound::create("data-files/sounds/ui_laser_shoot_01.wav"),			// 6  = shock wave
		Sound::create("data-files/sounds/ui_wood_close.wav"),				// 7  = mountain
		Sound::create("data-files/sounds/ui_wood_close.wav"),				// 8  =
		Sound::create("data-files/sounds/ui_wood_back.wav"),				// 9  = 
		Sound::create("data-files/sounds/ui_casual_pops_back.wav"),			// 10 = 
		Sound::create("data-files/sounds/ui_casual_pops_close.wav")			// 11 = 
	);

}

void App::initializeModel() {
    ArticulatedModel::Part* part = m_model->addPart("root");
}


// Adds given model to scene and returns a visible entity. Entity is not visible by default
shared_ptr<VisibleEntity> App::addEntity(shared_ptr<ArticulatedModel> model, String entityName, bool visible) {
    //TODO understand and fix this method.

    // Replace any existing voxel model. Models don't 
    // have to be added to the model table to use them 
    // with a VisibleEntity.
    ModelTable mt = scene()->modelTable();
    String name = model->name();

    if (mt.containsKey(name)){
        scene()->removeModel(model->name());
    }
    scene()->insert(model);

    // Replace any existing voxel entity that has the wrong type
    shared_ptr<Entity> entity = scene()->entity(entityName);
    if (notNull(entity) && isNull(dynamic_pointer_cast<VisibleEntity>(entity))) {
        logPrintf("The scene contained an Entity named %s that was not a VisibleEntity\n", entityName);
        scene()->remove(entity);
        entity.reset();
    }

    if (isNull(entity)) {
        // There is no voxel entity in this scene, so make one.
        //
        // We could either explicitly instantiate a VisibleEntity or simply
        // allow the Scene parser to construct one. The second approach
        // has more consise syntax for this case, since we are using all constant
        // values in the specification.
        Any anyFile = 
            PARSE_ANY(
                VisibleEntity {
                    model = "";
                    shouldBeSaved = false;
                    canChange = true;
                };
            );

        anyFile["model"] = model->name();
        entity = scene()->createEntity(entityName, anyFile);

    } else {
        // Change the model on the existing  entity
        dynamic_pointer_cast<VisibleEntity>(entity)->setModel(model);
    }

    shared_ptr<VisibleEntity> visibleEntity = scene()->typedEntity<VisibleEntity>(entityName);
    visibleEntity->setVisible(visible);
    return visibleEntity;
}

void App::removeEntity(shared_ptr<VisibleEntity> entity){
    scene()->removeEntity(entity->name());
}


//void App::changeSkymap(String filename){
//    shared_ptr<Skybox> skybox = scene()->typedEntity<Skybox>("environmentMap");
//    skybox = 
//
//}

// Returns true if the positions given is occupied.
bool App::voxIsSet(Point3int32 pos) {
    return getChunk(pos)->containsKey(pos);
}


// Returns the chunk coords for a given point.
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

/** Used explicitly for removing voxels, check whether the voxel is the boundary of a chunk.
	If it is, push the neighboring chunk to the m_chunksToRedraw for later update. 
	Can add up to 2 more chunks for update. */
void App::checkBoundaryAdd(Point3int32 pos){
    Point2int32 current = getChunkCoords(pos);
    Point2int32 xPlus   = getChunkCoords(pos + Point3int32(1,0,0));
    Point2int32 xMinus  = getChunkCoords(pos + Point3int32(-1,0,0));
    Point2int32 zPlus   = getChunkCoords(pos + Point3int32(0,0,1));
    Point2int32 zMinus  = getChunkCoords(pos + Point3int32(0,0,-1));

    if ( (xPlus != current) && (!m_chunksToUpdate.contains(xPlus)) ) {
        m_chunksToUpdate.push(xPlus);
    } 
    if ( (xMinus != current) && (!m_chunksToUpdate.contains(xMinus)) ) {
        m_chunksToUpdate.push(xMinus);
    }
    if ( (zPlus != current) && (!m_chunksToUpdate.contains(zPlus)) ) {
        m_chunksToUpdate.push(zPlus);
    } 
    if ( (zMinus != current) && (!m_chunksToUpdate.contains(zMinus)) ) {
        m_chunksToUpdate.push(zMinus);
    }
}

// Return the chunk a given voxel resides in.
shared_ptr<Table<Point3int32, int>> App::getChunk(Point3int32 pos) {
    Point2int32 key = getChunkCoords(pos);
    if (!m_posToChunk.containsKey(key)) {
        m_posToChunk.set( key, std::make_shared<Table<Point3int32, int>>(Table<Point3int32, int>()) );
    }
    return m_posToChunk[key];
}

// Return the voxel type at a given grid position.
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

// Clear the geometry for a given chunk.
void App::clearChunk(Point2int32 chunkPos) {
    // Clear CPU vertex and CPU index arrays for every chunk type
    for (int i = 0; i < voxTypeCount; i++) {
	    if ( notNull(m_model->geometry(format("geom %d,%d,%d", chunkPos.x, chunkPos.y, i))) ) {
            ArticulatedModel::Geometry* geometry = m_model->geometry(format("geom %d,%d,%d", chunkPos.x, chunkPos.y, i ));
            ArticulatedModel::Mesh*     mesh     = m_model->mesh(format("mesh %d,%d,%d", chunkPos.x, chunkPos.y, i ));

	        Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	        Array<int>&					   indexArray  = mesh->cpuIndexArray;

            vertexArray.fastClear();
	        indexArray.fastClear();

            ArticulatedModel::CleanGeometrySettings geometrySettings;
            geometrySettings.allowVertexMerging = false;
            m_model->cleanGeometry(geometrySettings);

            geometry->clearAttributeArrays();
            mesh->clearIndexStream();
       
	        vertexArray.fastClear();
	        indexArray.fastClear();
        
            //hasValue doesn't work so I made this thing from the document:
            bool hasSomething = false;
            for(Table<Point3int32, int>::Iterator it = m_posToChunk[chunkPos]->begin();it.isValid();++it){
                 if ((*it).value == i) {
                     hasSomething = true;
                 }
            
            
            }


			//A TERRIBLE WORK AROUND
			if (!hasSomething){
			    for (int i = 0; i < 3; i++) {
			        CPUVertexArray::Vertex& dummy = vertexArray.next();
			        dummy.position = Point3(0,0,0);
			        dummy.texCoord0 = Point2(0, 0);
			        dummy.normal  = Vector3::nan();
			        dummy.tangent = Vector4::nan();
			    }
			    indexArray.append(0,1,2);
			    
			}
        }  
    }
}

// Redraw the geometry for a given chunk.
void App::createChunkGeometry(Point2int32 chunkPos) {
	
    for (Table<Point3int32, int>::Iterator it = m_posToChunk[chunkPos]->begin(); it.hasMore(); ++it) {
	    createVoxelGeometry( it->key );
	}
    
	// Update the geometry for every type
    for (int i = 0; i < voxTypeCount; i++) { 
        updateGeometry(chunkPos, i);
    }

    int index = m_chunksToUpdate.findIndex(chunkPos);
    if (index > -1) {
        m_chunksToUpdate.remove(index);
    }
    scene()->entity("voxel")->markChanged();
}

void App::updateChunks() {
    for (int i = 0; i < m_chunksToUpdate.size(); ++i) {        
        clearChunk(m_chunksToUpdate[i]);
        createChunkGeometry(m_chunksToUpdate[i]);
    }
}

//Redraw the geometry for the entire voxel world.
void App::redrawWorld() {
    Array<Point2int32> chunkArray = m_posToChunk.getKeys();
    for (int i = 0; i < chunkArray.size(); ++i) {
        clearChunk(chunkArray[i]);
        createChunkGeometry(chunkArray[i]);

    }
}

 

// Input = Center of vox
void App::addVoxel(Point3int32 input, int type) {
    setVoxel(input, type);
    createVoxelGeometry(input);
    updateGeometry(getChunkCoords(input),type);
	m_sounds[0]->play();
}


void App::createVoxelGeometry(Point3int32 input) {
    int type = posToVox(input);
	ArticulatedModel::Geometry* geometry;
	ArticulatedModel::Mesh*     mesh;

	Point2int32 chunkCoords = getChunkCoords(input);
	if ( isNull(m_model->geometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type))) ) {
		geometry = m_model->addGeometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type));
		mesh	 = m_model->addMesh(format("mesh %d,%d,%d", chunkCoords.x, chunkCoords.y, type), m_model->part("root"), geometry);
		mesh->material = m_voxToMat[type];
	}
    geometry = m_model->geometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type));
    mesh	 = m_model->mesh(format("mesh %d,%d,%d", chunkCoords.x, chunkCoords.y, type));

	Point3 pos = Util::voxelToWorldSpace(input);

	// Check each position adjacent to voxel, and if nothing is there, add a face
    if ( !voxIsSet(input + Vector3int32(1,0,0)) ) {
        addFace(geometry, mesh, pos, voxelRes, Vector3int32(1,0,0), Vector3::X_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(-1,0,0)) ) {
        addFace(geometry, mesh, pos, voxelRes, Vector3int32(-1,0,0), Vector3::X_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(0,1,0)) ) {
        addFace(geometry, mesh, pos, voxelRes, Vector3int32(0,1,0), Vector3::Y_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(0,-1,0)) ) {
        addFace(geometry, mesh, pos, voxelRes, Vector3int32(0,-1,0), Vector3::Y_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(0,0,1)) ) {
        addFace(geometry, mesh, pos, voxelRes, Vector3int32(0,0,1), Vector3::Z_AXIS, type);
    }
    if ( !voxIsSet(input + Vector3int32(0,0,-1)) ) {
        addFace(geometry, mesh, pos, voxelRes, Vector3int32(0,0,-1), Vector3::Z_AXIS, type);
    }
    
}

/** Used for making the voxels in the menu and hand models, doesn't cull */
void App::drawVoxelNaive(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, int type) {
	addFace(geometry, mesh, pos, size, Vector3int32(1,0,0),  Vector3::X_AXIS, type);
	addFace(geometry, mesh, pos, size, Vector3int32(-1,0,0), Vector3::X_AXIS, type);
	addFace(geometry, mesh, pos, size, Vector3int32(0,1,0),  Vector3::Y_AXIS, type);
	addFace(geometry, mesh, pos, size, Vector3int32(0,-1,0), Vector3::Y_AXIS, type);
	addFace(geometry, mesh, pos, size, Vector3int32(0,0,1),  Vector3::Z_AXIS, type);
	addFace(geometry, mesh, pos, size, Vector3int32(0,0,-1), Vector3::Z_AXIS, type);
}

void App::addFace(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, Vector3 normal, Vector3::Axis axis, int type) {
	// Center of face we are adding
	Point3 center = pos + normal * (0.5f * size);

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
	a.position = center + ((u * 0.5f - v * 0.5f) * size);
    a.texCoord0=Point2(1, 0);
    a.normal  = Vector3::nan();
    a.tangent = Vector4::nan();

	CPUVertexArray::Vertex& b = vertexArray.next();
	b.position = center + ((u * 0.5f + v * 0.5f) * size);
    b.texCoord0 = Point2(1, 1);
    b.normal  = Vector3::nan();
    b.tangent = Vector4::nan();

	CPUVertexArray::Vertex& c = vertexArray.next();
	c.position = center + ((-u * 0.5f + v * 0.5f) * size);
    c.texCoord0 = Point2(0, 1);
    c.normal  = Vector3::nan();
    c.tangent = Vector4::nan();

	CPUVertexArray::Vertex& d = vertexArray.next();
	d.position = center + (( -u * 0.5f - v * 0.5f) * size);
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

}

// Clean/update the geometry for our model.
void App::updateGeometry(Point2int32 chunkCoords, int type) {
    
    if ( notNull(m_model->geometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type))) ) {
		ArticulatedModel::Geometry* geometry = m_model->geometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type));
        ArticulatedModel::Mesh*     mesh	 = m_model->mesh(format("mesh %d,%d,%d", chunkCoords.x, chunkCoords.y, type));
        // Automatically compute normals. get rid of this when figure out how to modify gpuNormalArray   ????
	    ArticulatedModel::CleanGeometrySettings geometrySettings;
        geometrySettings.allowVertexMerging = false;
        m_model->cleanGeometry(geometrySettings);

	    // If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
	    geometry->clearAttributeArrays();

	    // If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
    	mesh->clearIndexStream();

	}

}

void App::removeVoxel(Point3int32 input) {
	unsetVoxel(input);

	Point2int32 chunkCoords = getChunkCoords(input);
    if( !m_chunksToUpdate.contains(chunkCoords) ) {
        m_chunksToUpdate.push(chunkCoords);
    }

    checkBoundaryAdd(input);

	m_sounds[1]->play();
}




void App::cameraIntersectVoxel(Point3int32& lastPos, Point3int32& hitPos){
    
	// Intersect with empty space
    const float maxDist = 2.0f + intersectMode * 10.0f;
    Vector3 direction = m_crosshair.lookDirection;
    
    Ray cameraRay(m_crosshair.position, m_crosshair.lookDirection);

    //Point2 center = UserInput(this->window()).mouseXY();
    //Ray cameraRay = activeCamera()->worldRay(center.x / this->window()->width() * renderDevice->width(), center.y / this->window()->height() * renderDevice->height(), renderDevice->viewport());

    hitPos = Point3int32( (m_crosshair.position + m_crosshair.lookDirection * maxDist) / voxelRes );

    lastPos = hitPos;

	// Intersect with voxel
    if(!forceIntersect){
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
}

void App::debugDrawVoxel(){

	addEntity(m_debugModel, "debugEntity");
	ArticulatedModel::Part* part = m_debugModel->addPart("root");
	int type = voxTypeCount;
	
	ArticulatedModel::Geometry* geometry;
	ArticulatedModel::Mesh*     mesh;
	
	if (isNull(m_debugModel->geometry("geom")))  {
		geometry = m_debugModel->addGeometry("geom");
	    mesh	 = m_debugModel->addMesh("mesh", m_debugModel->part("root"), geometry);
	 
	} else {
	    geometry = m_debugModel->geometry("geom");
	    mesh	 = m_debugModel->mesh("mesh");
	}
	
	mesh->material = m_voxToMat[type];
	Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	Array<int>&					   indexArray  = mesh->cpuIndexArray;
	
	vertexArray.fastClear();
	indexArray.fastClear();
	Array<Point3int32>& selectionArray = m_selection.getArray();

	for (int i = 0; i < selectionArray.length(); i++) {
	    Point3int32 pos= selectionArray[i];
	    Point3 selection = Util::voxelToWorldSpace(pos);
	    drawVoxelNaive(geometry, mesh, selection, voxelRes * 1.1f, type);
	}
	
	//yet another bad workaround
	if (selectionArray.length() == 0) {
		drawVoxelNaive(geometry, mesh, Point3(0,0,0), 0, type);
	}
	
	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	m_debugModel->cleanGeometry(geometrySettings);
	
	// If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
	geometry->clearAttributeArrays();
	
	// If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
	mesh->clearIndexStream();
}

void App::selectBox(Point3int32 center, int radius) {
    m_selection.clear();

    for (int y = center.y - radius; y <= center.y + radius; ++y) {
        for (int x = center.x - radius; x <= center.x + radius; ++x) {
            for (int z = center.z - radius; z <= center.z + radius; ++z) {
                Point3int32 pos = Point3int32(x, y, z);

				if ( voxIsSet(pos) ) {
                    m_selection.getArray().append(pos);
				}
            }
        }
    }

    debugDrawVoxel();
}

void App::selectCylinder(Point3int32 center, int radius) {
    m_selection.clear();

    for (int y = center.y - radius; y <= center.y + radius; ++y) {
        for (int x = center.x - radius; x <= center.x + radius; ++x) {
            for (int z = center.z - radius; z <= center.z + radius; ++z) {
                Point3int32 pos = Point3int32(x, y, z);

				// check if the voxel is in the cylinder
                if(sqrt((x-center.x) * (x-center.x) + (z-center.z) * (z-center.z)) <= radius && voxIsSet(pos)) {
                    m_selection.getArray().append(pos);
                }
            }
        }
    }

    debugDrawVoxel();
}

void App::pullVoxelOrbit(Point3int32 origin) {

    Array<Point3int32> queue;
    queue.push(origin);
    while(queue.size()) {
        
    }

    std::function<void (SimTime, SimTime, Table<String, float>)> lambda = [&](SimTime sdt, SimTime st, Table<String, float> args) {
        
    };

}

void App::makeCrater(Point3int32 center, int radius) {

	// Lambda function for AnimationControl.
	// sdt = differential (time since last call of this function)
	// st = absolute (time since animation began)
	
	std::function<void (SimTime, SimTime, shared_ptr<Table<String, float>>, Table< Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&)> lambda = [&](SimTime sdt, SimTime st, shared_ptr<Table<String, float>> args, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&) {

		int currentRadius = (int)(args->get("currentRadius"));
		int radius = (int)args->get("radius");
		Point3int32 center(args->get("centerX"), args->get("centerY"), args->get("centerZ"));

		SimTime threshold = 0.005f * currentRadius;

		if ( currentRadius < radius ) {
			if ( st > threshold ) {
				float bound = max(currentRadius / sqrt(2.0f) - 1.0f, 0.0f);
				float depth = sqrt(radius - currentRadius);
				
				for (Point3int32 P(center.x, center.y - depth, center.z); P.y <= center.y; ++P.y) {
					for (P.x = center.x - currentRadius; P.x <= center.x - bound; ++P.x) {
						for (P.z = center.z - currentRadius; P.z <= center.z + currentRadius; ++P.z) {
					
							if ( sqrt((P.x - center.x) * (P.x - center.x) + (P.z - center.z) * (P.z - center.z)) <= currentRadius && voxIsSet(P) ) {
								removeVoxel(P);
							}
					
						}
					}
				}

				for (Point3int32 P(center.x, center.y - depth, center.z); P.y <= center.y; ++P.y) {
					for (P.x = center.x + bound; P.x <= center.x + currentRadius; ++P.x) {
						for (P.z = center.z - currentRadius; P.z <= center.z + currentRadius; ++P.z) {
					
							if ( sqrt((P.x - center.x) * (P.x - center.x) + (P.z - center.z) * (P.z - center.z)) <= currentRadius && voxIsSet(P) ) {
								removeVoxel(P);
							}
					
						}
					}
				}

				for (Point3int32 P(center.x, center.y - depth, center.z); P.y <= center.y; ++P.y) {
					for (P.x = center.x - bound; P.x <= center.x + bound; ++P.x) {
						for (P.z = center.z - currentRadius; P.z <= center.z - bound; ++P.z) {

							if ( sqrt((P.x - center.x) * (P.x - center.x) + (P.z - center.z) * (P.z - center.z)) <= currentRadius && voxIsSet(P) ) {
								removeVoxel(P);
							}
					
						}
					}
				}

				for (Point3int32 P(center.x, center.y - depth, center.z); P.y <= center.y; ++P.y) {
					for (P.x = center.x - bound; P.x <= center.x + bound; ++P.x) {
						for (P.z = center.z + bound; P.z <= center.z + currentRadius; ++P.z) {
					
							if ( sqrt((P.x - center.x) * (P.x - center.x) + (P.z - center.z) * (P.z - center.z)) <= currentRadius && voxIsSet(P) ) {
								removeVoxel(P);
							}
					
						}
					}
				}

				currentRadius++;
				args->set("currentRadius", float(currentRadius));
			}
		}

		if (currentRadius == radius) {
			lastAnimFinished = true;
		} else {
			lastAnimFinished = false;
		}
			
	};


	AnimationControl a(lambda);
	a.args->set("radius", float(radius));
	a.args->set("currentRadius", 0.0f);
	a.args->set("centerX", float(center.x));
	a.args->set("centerY", float(center.y));
	a.args->set("centerZ", float(center.z));
	m_animControls.push(a);

}

void App::makeShockWave(Point3 origin, Vector3 direction) {
    debugPrintf("SHOCKWAVE START\n");
    std::function<void (SimTime, SimTime, shared_ptr<Table<String, float>>, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&)> lambda = [&](SimTime sdt, SimTime st, shared_ptr<Table<String, float>> args, Table<Point3int32, shared_ptr<VisibleEntity>>& entities, Table<Point3int32, SimTime>& timeAdded) {

        float bounds = (st / 12.0f) * (float)(1<<7);

        //the Boundary of the voxels that would intersect
		Point3int32 voxelBound = Point3int32( (int)bounds, (int)bounds, (int)bounds );


        Point3 origin = Point3(args->get("originX"),args->get("originY"), args->get("originZ"));
		Vector3 direction = Vector3(args->get("directionX"), args->get("directionY"), args->get("directionZ"));

        origin = Point3(0,-1,0);

        //debugPrintf("ORIGIN> %f, %f, %f\n", origin.x, origin.y, origin.z);
        //debugPrintf("DIRECTION> %f, %f, %f\n", direction.x, direction.y, direction.z);
        Ray shockWaveRay(origin, direction);


        int i = 0;
        int j = 0;
        int numVoxels = 50;
		for (RayGridIterator it(shockWaveRay, voxelBound, Vector3(voxelRes,voxelRes,voxelRes), Point3(-voxelBound / 2) * voxelRes, -voxelBound / 2); it.insideGrid() && i < numVoxels; ++it) {
            //debugPrintf("%d: %d %d %d\n", j, it.index().x, it.index().y, it.index().z);
            j++;
            if(voxIsSet(it.index())){
                shared_ptr<VisibleEntity> ent;
                SimTime timePassed;
                String name = format("animatedvox%d,%d,%d", it.index().x, it.index().y, it.index().z);

                if(!entities.containsKey(it.index())){
                    shared_ptr<ArticulatedModel> model = makeVoxelModel( name, posToVox( it.index() ) );    
                    ent = addEntity(model, name, true);
                    entities.set(it.index(), ent);
                    timeAdded.set(it.index(), st);
                    timePassed = 0.0f;
                }else{
                    //debugPrintf("EXISTING ENT\n");
                    ent = entities[it.index()];
                    timePassed = (st - timeAdded[it.index()]);

                }

                    float rise;
                    float totalTime = 1.0f;
                    float maxRise = 1.0f;
                    if(timePassed <= totalTime/2.0f){
                        rise = (timePassed/(totalTime/2.0f))*maxRise;
                    }else if(timePassed <= totalTime){
                        rise = ( ( (totalTime/2.0f) - (timePassed - (totalTime/2.0f) ) ) / (totalTime/2.0f))*maxRise;
                    }else{
                        ent->setVisible(false);

                        if(notNull(scene()->entity(ent->name()))){
                            removeEntity(ent);   
                        }
                    }
                    Point3int32 voxPos = Point3int32(it.index().x, it.index().y, it.index().z);
                    Point3 worldPos = Util::voxelToWorldSpace(voxPos);
                    CoordinateFrame frame = CFrame::fromXYZYPRRadians(worldPos.x, worldPos.y + rise, worldPos.z);
                    ent->setFrame(frame);
                    ++i;
                }else{
                
                break;

            }
            
        }

        lastAnimFinished = st > 30.0f;
        //printf("finishFrame\n");

        if(lastAnimFinished){
            //Array<shared_ptr<VisibleEntity>> ents;
            //entities.getValues(ents);
            //for(int i = 0; i < ents.size(); ++i){
            //
            //    if(notNull(scene()->entity(ents[i]->name()))){
            //        removeEntity(ents[i]);   
            //    }
            //}
        
            entities.clear();
        }
    };
    AnimationControl a(lambda);
	a.args->set("originX", origin.x);
	a.args->set("originY", origin.y);
	a.args->set("originZ", origin.z);
	a.args->set("directionX", direction.x);
	a.args->set("directionY", direction.y);
	a.args->set("directionZ", direction.z);
	m_animControls.push(a);

}

void App::makeMountain(Point3int32 center, int radius, int height) {

	// Lambda function for AnimationControl.
	// sdt = differential (time since last call of this function)
	// st = absolute (time since animation began)
	
	std::function<void (SimTime, SimTime, shared_ptr<Table<String, float>>, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&)> lambda = [&](SimTime sdt, SimTime st, shared_ptr<Table<String, float>> args, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&) {

		int currentRadius = (int)(args->get("currentRadius"));
		int radius = (int)args->get("radius");
		int currentHeight = (int)(args->get("currentHeight"));
		int height = (int)args->get("height");
		Point3int32 center(args->get("centerX"), args->get("centerY"), args->get("centerZ"));

		SimTime threshold = 0.005f * currentRadius;

		if ( currentHeight < height ) {
			if ( st > threshold ) {
				float bound = max(currentRadius / sqrt(2.0f) - 1.0f, 0.0f);
				float depth = sqrt(radius - currentRadius);
				
				for (Point3int32 P(center.x, center.y - depth, center.z); P.y <= center.y; ++P.y) {
					for (P.x = center.x - currentRadius; P.x <= center.x - bound; ++P.x) {
						for (P.z = center.z - currentRadius; P.z <= center.z + currentRadius; ++P.z) {
					
							if ( sqrt((P.x - center.x) * (P.x - center.x) + (P.z - center.z) * (P.z - center.z)) <= currentRadius && voxIsSet(P) ) {
								removeVoxel(P);
							}
					
						}
					}
				}

				for (Point3int32 P(center.x, center.y - depth, center.z); P.y <= center.y; ++P.y) {
					for (P.x = center.x + bound; P.x <= center.x + currentRadius; ++P.x) {
						for (P.z = center.z - currentRadius; P.z <= center.z + currentRadius; ++P.z) {
					
							if ( sqrt((P.x - center.x) * (P.x - center.x) + (P.z - center.z) * (P.z - center.z)) <= currentRadius && voxIsSet(P) ) {
								removeVoxel(P);
							}
					
						}
					}
				}

				for (Point3int32 P(center.x, center.y - depth, center.z); P.y <= center.y; ++P.y) {
					for (P.x = center.x - bound; P.x <= center.x + bound; ++P.x) {
						for (P.z = center.z - currentRadius; P.z <= center.z - bound; ++P.z) {

							if ( sqrt((P.x - center.x) * (P.x - center.x) + (P.z - center.z) * (P.z - center.z)) <= currentRadius && voxIsSet(P) ) {
								removeVoxel(P);
							}
					
						}
					}
				}

				for (Point3int32 P(center.x, center.y - depth, center.z); P.y <= center.y; ++P.y) {
					for (P.x = center.x - bound; P.x <= center.x + bound; ++P.x) {
						for (P.z = center.z + bound; P.z <= center.z + currentRadius; ++P.z) {
					
							if ( sqrt((P.x - center.x) * (P.x - center.x) + (P.z - center.z) * (P.z - center.z)) <= currentRadius && voxIsSet(P) ) {
								removeVoxel(P);
							}
					
						}
					}
				}

				currentHeight++;
				args->set("currentHeight", float(currentHeight));

				if ( currentRadius > 1 ) {
					currentRadius--;
					args->set("currentRadius", float(currentRadius));
				}
			}
		}

		if (currentRadius == radius) {
			lastAnimFinished = true;
		} else {
			lastAnimFinished = false;
		}
			
	};


	AnimationControl a(lambda);
	a.args->set("currentRadius", float(radius));
	a.args->set("radius", float(radius));
	a.args->set("currentHeight", 0.0f);
	a.args->set("height", float(height));
	a.args->set("centerX", float(center.x));
	a.args->set("centerY", float(center.y));
	a.args->set("centerZ", float(center.z));
	m_animControls.push(a);

}

void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt){
     GApp::onSimulation(rdt, sdt, idt);

	 for (int i = 0; i < m_animControls.size(); ++i) {
		m_animControls[i].invoke(sdt);
		if (lastAnimFinished) {
			m_animControls.remove(i);
		}
	 }
}


bool App::onEvent(const GEvent& event) {
    
    if (super::onEvent(event)) { return true; }

    if (event.isMouseEvent() && event.button.type == GEventType::MOUSE_BUTTON_CLICK) {
          // Add voxel
          if (event.button.button == (uint8)0) {
            Point3int32 hitPos;
            Point3int32 lastPos;
            cameraIntersectVoxel(lastPos, hitPos);
            addVoxel(lastPos, m_voxelType);
          
          // Remove voxel
          } else if (event.button.button == (uint8)1) {
            Point3int32 hitPos;
            Point3int32 lastPos;
            cameraIntersectVoxel(lastPos, hitPos);
            removeVoxel(hitPos);
          }
    }

    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('m')) ){ 
            menuMode = !menuMode;
                
            if (menuMode) {
                menuFrame = activeCamera()->frame();
                m_menu->setFrame(menuFrame);
				m_sounds[3]->play();
            } else {
				m_sounds[4]->play();
			}

            m_menu->setVisible(menuMode);

    }  else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('o')) ){ 
        Point3int32 hitPos;
        Point3int32 lastPos;
        m_crosshair.lookDirection = Vector3(0,-1,0);
        cameraIntersectVoxel(lastPos, hitPos);
        updateSelect();
        Vector3 direction = Vector3(m_crosshair.lookDirection.x, 0.0f, m_crosshair.lookDirection.z);

        if(lastPos != hitPos){
            makeShockWave(hitPos, direction);
        }
    }

    // Change intersect distance
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('r')) ){ 
            intersectMode +=1;
            intersectMode %=3;
    } 

    // Force intersect
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('f')) ){ 
            forceIntersect = !forceIntersect;
    }

	// Box select
	else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('j')) ){ 
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
		m_currentMark = hitPos;
    }
	else if ( (event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('j')) ){
		// determine how big the radius is
		Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        selectBox(m_currentMark, Vector3(hitPos.x - m_currentMark.x, hitPos.y - m_currentMark.y, hitPos.z - m_currentMark.z).magnitude());
	}

	// Cylinder select
	else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('k')) ){ 
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
		m_currentMark = hitPos;
    }
	else if ( (event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('k')) ){
		// determine how big the radius is
		Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        selectCylinder(m_currentMark, Vector3(hitPos.x - m_currentMark.x, hitPos.y - m_currentMark.y, hitPos.z - m_currentMark.z).magnitude());
	}

	// Sphere select
	else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('l')) ){ 
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        m_selection.setMark(hitPos);
    }
	else if ( (event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('l')) ){
		// determine how big the radius is
        selectSphere(m_crosshair.position, m_crosshair.lookDirection);
	}

	// Elevate selection
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

	// Mountain
	else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('i')) ) {
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        m_currentMark = hitPos;
    }
	else if ( (event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('i')) ) {
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        makeMountain(hitPos.y - m_currentMark.y);
    }

	// Crater
	else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('p')) ){ 
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
		m_currentMark = hitPos;
    }
	else if ( (event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('p')) ){
		// determine how big the radius is
		Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        makeCrater(m_currentMark, Vector3(hitPos.x - m_currentMark.x, hitPos.y - m_currentMark.y, hitPos.z - m_currentMark.z).magnitude());
	}

    //else if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) {
    //    m_firstPersonMode = ! m_firstPersonMode;
    //    const shared_ptr<Camera>& camera = m_firstPersonMode ? scene()->defaultCamera() : debugCamera();
    //    setActiveCamera(camera);
    //}
    //
    //else if (event.key.keysym.sym == GKey('w')){
    //    if((event.type == GEventType::KEY_DOWN)){
    //        player.desiredOS = Vector3(player.speed, 0, 0);
    //    }else{
    //        player.desiredOS = Vector3(0,0,0);
    //    }
    //}

    // Handle super-class events

    return false;
}

void App::selectSphere(Point3 origin, Vector3 direction) {
    m_selection.commitSphere(origin, direction);
    debugDrawVoxel();
}

void App::elevateSelection(int delta) {
    Array<Point3int32>& selectionArray = m_selection.getArray();
    Array<Point3int32> changeBuffer;
    if (delta != 0) {
        for (int i = 0; i < selectionArray.size(); ++i) {
            if(voxIsSet(selectionArray[i])){
                changeBuffer.push(selectionArray[i]);
            }
        }

        for(int i = 0; i < changeBuffer.size(); ++i) {
            Point3int32 targetPos = changeBuffer[i] + Point3int32(0, delta, 0);
            setVoxel(targetPos, posToVox(changeBuffer[i]));
            Point2int32 chunkCoords = getChunkCoords(changeBuffer[i]);
            if( !m_chunksToUpdate.contains(chunkCoords) ) {
                m_chunksToUpdate.push(chunkCoords);
            }
            unsetVoxel(selectionArray[i]);
        }
    }
    m_selection.clear();
    debugDrawVoxel();

	m_sounds[2]->play();
}

void App::onUserInput(UserInput* ui) {
    
    super::onUserInput(ui);

    if(!vrEnabled){
        updateSelect();
    }
    
}

void App::onGraphics(RenderDevice * rd, Array< shared_ptr< Surface > > & surface, Array< shared_ptr< Surface2D > > & surface2D ) {

    updateChunks();
    if(menuMode){
        CFrame frame = menuFrame;
        debugDrawLabel(frame.pointToWorldSpace(m_menuButtons[voxTypeCount]), Vector3(0,0,0), GuiText("Select Voxel Type"));
    }
    if(vrEnabled){
        //updateSelect();
        //Point3 head;
        //Point3 hand1;
        //Point3 hand2;
        //if(m_vrControllerArray.size() > 0){
        //    hand1 = m_vrControllerArray[0]->frame().pointToWorldSpace(Point3(0,0,0));
        //    debugDraw(Sphere(hand1, 0.1), 0.0f, Color3::blue());
        //}
        //if(m_vrControllerArray.size() > 1){
        //    hand2 = m_vrControllerArray[1]->frame().pointToWorldSpace(Point3(0,0,0));
        //    debugDraw(Sphere(hand2, 0.1), 0.0f, Color3::orange());
        //}
        //
        //
        //
        //if(menuMode){
        //   CFrame frame = menuFrame;
        //   //m_menu->setVisible(true);
        //}
        //
        //
        //
        ////VR CONTROLLER INPUT
        ////BEGIN_PROFILER_EVENT("VRApp::sampleTrackingData");
        //
        //vr::VREvent_t vrEvent;
        //while (m_hmd->PollNextEvent(&vrEvent, sizeof(vrEvent))) {
        //    debugPrintf("Device %d sent button %d press\n", vrEvent.trackedDeviceIndex, vrEvent.data.controller.button);
        //    switch (vrEvent.eventType) {
        //    // Device 0 is the HMD (probably). Device 1 and 2 are the controllers (probably) on Vive.
        //    // Button 32 = k_EButton_Axis0 is the large DPad/touch button on the Vive.
        //    case vr::VREvent_ButtonPress:
        //        if(!menuMode){
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
        //                if(m_crosshair.buttonSelected){
        //                    m_voxelType = m_crosshair.buttonIndex;
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
        //            menuMode = !menuMode;
        //            debugPrintf("MENU %d\n", menuMode);
        //            if(menuMode){
        //                float yaw, pitch, roll;
        //                m_vrHead->frame().rotation.toEulerAnglesXYZ(yaw, pitch, roll);
        //                Point3 translation = m_vrHead->frame().translation;
        //                menuFrame = CoordinateFrame::fromXYZYPRDegrees(translation.x, translation.y, translation.z, yaw);
        //                m_crosshair.menuControllerIndex = (vrEvent.trackedDeviceIndex - 1) % 2;
        //                m_menu->setFrame(menuFrame);
        //            }
        //            m_menu->setVisible(menuMode);
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
        //
        ////END_PROFILER_EVENT();
        
        
        


    }

    super::onGraphics(rd, surface, surface2D);


}
