/** \file App.cpp */
#include "App.h"
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
    //GApp::Settings settings(argc, argv);
    //////////////////////////////
    
    // VR CODE////////////////


   // VR CODE////////////////
	VRApp::Settings settings(argc, argv);
	settings.vr.debugMirrorMode = //VRApp::DebugMirrorMode::NONE;//
	VRApp::DebugMirrorMode::PRE_DISTORTION;
	
	 settings.vr.disablePostEffectsIfTooSlow = true;
	 settings.renderer.deferredShading   = true;
	 settings.renderer.orderIndependentTransparency = true;
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

    debugWindow->setVisible(false);
    developerWindow->videoRecordDialog->setEnabled(true);
 
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));

    debugPane->beginRow(); {
        GuiTabPane* containerPane = debugPane->addTabPane();

        GuiPane* pane1 = containerPane->addTab("Basic controls");
		pane1->addLabel("Left click: Add voxel");
		pane1->addLabel("Middle click: Remove voxel");
        pane1->addLabel("Mouse scroll: Change intersection distance");
		pane1->addLabel("f: Toggle force intersect");
		pane1->addLabel("Change voxel types by using the number keys 0 - 7.");
		m_typesList.append("Grass", "Rock", "Brick", "Sand", "Rough Cedar", "Rusty Metal");
		m_typesList.append("Chrome", "Black Rubber");
        pane1->addDropDownList("Voxel Type", m_typesList, &m_voxelType);
		pane1->pack();

        GuiPane* pane2 = containerPane->addTab("Selection controls");
		pane2->addLabel("Hold and release keys. Center of selection is determined on key press. Radius is determined on release.");
		pane2->addLabel("n: Change intersection mode (regular, union, minus, intersection)");
		pane2->addLabel("j: Box select");
		pane2->addLabel("k: Cylinder select");
		pane2->addLabel("l: Sphere select");
		pane2->addLabel("u: Elevate selection. Hold, drag up to determine height, and release.");
		pane2->pack();

        GuiPane* pane3 = containerPane->addTab("Animation controls");
		pane3->addLabel("i: Mountain. Hold, drag up to determine height, and release.");
		pane3->addLabel("o: Shock wave");
		pane3->addLabel("p: Crater. Center of crater is determined on key press. Radius is determined on release.");
		pane3->addLabel("b: Orbit");
		pane3->pack();

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

    loadScene("Tero");

	initializeScene();

    updateCrosshair();

	setActiveCamera(m_debugCamera);

}

void App::getMenuCrosshairRay(){
    Ray cameraRay;
    Ray empty;
	int controllerIndex = 0;
	m_crosshair.buttonSelected = false;
    while(controllerIndex < 2 && !m_crosshair.buttonSelected){
		//if(m_vrControllerArray.size() > controllerIndex){
        //    cameraRay = m_vrControllerArray[ controllerIndex ]->frame().lookRay();
        //    cameraRay = Ray(cameraRay.origin(), cameraRay.direction());
        //}
		//
		//int numOptions = menuPageSize;
		//if(menuType == 1){
		//	numOptions = m_transforms.size();
		//}
		//for(int i = 0; i < numOptions && ! m_crosshair.buttonSelected; ++i){
		//    if(( cameraRay.origin() - menuFrame.pointToWorldSpace(m_menuButtonPositions[i]) ).length() <= 0.3){
		//         m_crosshair.buttonSelected = true;
		//         m_crosshair.voxelIndex = i + menuPageSize*m_currentMenuPage;
		//         m_crosshair.buttonIndex = i;
		//    }    
		//}
		//
		//if(cameraRay.origin() != empty.origin()){
		//     m_crosshair.lookDirection = cameraRay.direction();
		//     m_crosshair.position = cameraRay.origin();
		//     m_crosshair.ray = cameraRay;
		//     drawCrosshair();
		//}
	
		++controllerIndex;
	 }
	
}


Ray App::getVrCrosshairRay() {
    Ray crosshairRay;
   
	// vrEnabled
    //if(m_vrControllerArray.size() > 0){
    //    crosshairRay = m_vrControllerArray[0]->frame().lookRay();
    //}
   
   
    return crosshairRay;
}

CFrame App::getPlayerFrame(){
	if(vrEnabled){
	//	return m_vrHead->frame();
	}
	return activeCamera()->frame();

}


Ray App::getMouseCrosshairRay() {

    Point2 center = Point2( UserInput(this->window()).mouseXY().x / this->window()->width(), UserInput(this->window()).mouseXY().y / this->window()->height() );
    return activeCamera()->worldRay(center.x * renderDevice->viewport().width(), center.y * renderDevice->viewport().height(), renderDevice->viewport());

}

void App::updateCrosshairRay(Ray crosshairRay) {

	m_crosshair.lookDirection = crosshairRay.direction();
	m_crosshair.position = crosshairRay.origin();
	m_crosshair.ray = crosshairRay;
	drawCrosshair();
}

void App::updateCrosshair() {
    Ray empty;
    Ray crosshairRay;

    if ( menuMode ) {

        getMenuCrosshairRay();
    
    } else {
        if ( vrEnabled ) {
            crosshairRay = getVrCrosshairRay();
        } else {
            crosshairRay = getMouseCrosshairRay();
        }

        if ( crosshairRay.origin() != empty.origin() ) {
            updateCrosshairRay(crosshairRay);
        }
    }

}

void App::drawCrosshair() {

    if (menuMode) {
        if (m_crosshair.buttonSelected) {
            debugDraw(Sphere(menuFrame.pointToWorldSpace(m_menuButtonPositions[m_crosshair.buttonIndex]), 0.25), 0.0f, Color3::white());
        }
    } else {

        Point3int32 lastOpen;
        Point3int32 voxelTest;
        cameraIntersectVoxel(lastOpen, voxelTest);
        Point3 voxelHit = Util::voxelToWorldSpace(voxelTest);
        Point3 sideHit = Util::voxelToWorldSpace(lastOpen);
        Vector3 side = sideHit - voxelHit;

        sideHit = voxelHit + side * 0.01;

		float c = voxelRes / 2.0f;
		Point3 P = Point3(c, c, c);
        debugDraw( Box(voxelHit - P, voxelHit + P) );
        
		if (lastOpen != voxelTest) {
			c = voxelRes / 2.01f;
			P = Point3(c, c, c);
            debugDraw( Box(sideHit - P, sideHit + P), 0.0f, Color3::blue() );
        }
       
    }
}

void App::initializeScene() {
	// this is to set the number of types of voxels to only the manually added ones
    for (int i = 0; i < numTexturedVoxels; ++i) {
        m_voxToProp.append(Any::fromFile(format("data-files/voxelTypes/vox%d.Any", i)));
    }

    importVoxFile();

	initializeMaterials();

	initializeSounds();

    initializeModel();

    addEntity(m_model, "voxel");

    // Initialize ground
    Noise& rng = Noise::common();
    for(Point3int32 P(-100, 0, 0); P.x < 100; ++P.x) {
        for(P.z = -100; P.z < 100; ++P.z) {
            int height = (rng.sample(500 * P.x, 0, 500 * P.z) / 5000);
            for(P.y = -10; P.y < height; ++P.y) {
                setVoxel(P, grass);
            }
        }
    }

	numMenuPages = voxTypeCount / menuPageSize;
	getMenuPositions();
    makeMenuPageModels();
	makeMenuPageEntities();

    //if(vrEnabled){
    initializeHands();
    initializeTransforms();

    redrawWorld();
}

void App::initializeHands() {
	updateHands(0);
}

void App::updateHands(int index) {
    m_hand1 = addEntity(makeVoxelModel("handModel1", index, 0.1f), "hand1", true);
    m_hand2 = addEntity(makeVoxelModel("handModel2", index, 0.1f), "hand2", true);
}


void App::initializeTransforms() {
	m_transforms.append("crater", "shockwave", "mountain", "orbit", "elevate");
}


void App::getMenuPositions() {

    int rows = 3;
    int totalVoxels = menuPageSize;
    int blocksPerRow = totalVoxels / rows;
    float rowSeparation = 0.25;
    float menuRadius = 0.75;
    float menuWidth =  0.75 * PI; //radians
    float x, y, z;
    y = 0.75f;
    
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
        
        m_menuButtonPositions.append(Point3(x,y,z));
    }
    m_menuButtonPositions.append(Point3(0.0f, 0.75f, -menuRadius));
    
}

void App::makeMenuPageModels() {
	
	for (int pageNumber = 0; pageNumber < numMenuPages; ++pageNumber) {
	
		shared_ptr<ArticulatedModel> currentPageModel = ArticulatedModel::createEmpty(format("menuModel,%d", pageNumber));
		m_menuModels.append(currentPageModel);

		ArticulatedModel::Part* part = currentPageModel->addPart("root");

		int voxelsOnPage = min(menuPageSize, voxTypeCount - (menuPageSize * pageNumber));

		for (int t = 0; t < voxelsOnPage; ++t) {

			int currentType = menuPageSize * pageNumber + t;

			ArticulatedModel::Geometry* geometry = currentPageModel->addGeometry(format("geom %d", currentType));
			ArticulatedModel::Mesh*		mesh	 = currentPageModel->addMesh(format("mesh %d", currentType), currentPageModel->part("root"), geometry);
			mesh->material = m_voxToMat[currentType];

			Point3 current = m_menuButtonPositions[t];
			createNaiveVoxelGeometry(geometry, mesh, current, 0.25f, currentType);

		    ArticulatedModel::CleanGeometrySettings geometrySettings;
		    geometrySettings.allowVertexMerging = false;
		    currentPageModel->cleanGeometry(geometrySettings);

		    // If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
		    geometry->clearAttributeArrays();

		    // If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
			mesh->clearIndexStream();
		}
	}
}

void App::makeMenuPageEntities() {

	for (int pageNumber = 0; pageNumber < numMenuPages; ++pageNumber) {
		m_menu.append(addEntity(m_menuModels[pageNumber], format("menuEntity,%d", pageNumber), false));
	}

}

void App::changeMenuPage(int to) {
	if (menuMode) {
		m_menu[m_currentMenuPage]->setVisible(false);
		m_currentMenuPage = to;
		m_menu[m_currentMenuPage]->setVisible(true);
        m_menu[m_currentMenuPage]->setFrame(menuFrame);
		//setMenuFrame();

	}
}

void App::setMenuFrame(){
	//float yaw, pitch, roll;
	//m_vrHead->frame().rotation.toEulerAnglesXYZ(yaw, pitch, roll);
	//Point3 translation = m_vrHead->frame().translation;
	//menuFrame = CoordinateFrame::fromXYZYPRDegrees(translation.x, translation.y, translation.z, yaw);
	//menuFrame = m_vrHead->frame();
	//m_menu[m_currentMenuPage]->setFrame(menuFrame);

}

const shared_ptr<ArticulatedModel> App::makeVoxelModel(String modelName, int type, float size) {
	const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty(modelName);

    ArticulatedModel::Part*		part	 = model->addPart("root");
	ArticulatedModel::Geometry* geometry = model->addGeometry("geom");
	ArticulatedModel::Mesh*		mesh	 = model->addMesh("mesh", model->part("root"), geometry);
	mesh->material = m_voxToMat[type];
	
	createNaiveVoxelGeometry(geometry, mesh, Point3(0,0,0), size, type);
	
	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	model->cleanGeometry(geometrySettings);
	
	// If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
	geometry->clearAttributeArrays();
	
	// If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
	mesh->clearIndexStream();

    return model;
}

void App::importVoxFile(){
    //filesource seriously needs some help
    String fileSource = "data-files/voxelImport/monument/monu1.vox";
    BinaryInput voxInput(fileSource, G3D_LITTLE_ENDIAN);
    ParseVOX source;
    source.parse(voxInput);

	for (int i = 0; i < palSize; ++i) {
        TextOutput file1( format("data-files/texture/pal%d.UniversalMaterial.Any", i) );
        file1.printf("UniversalMaterial::Specification {lambertian = Color3(%f, %f, %f);}", float(source.palette[i].r), float(source.palette[i].g), float(source.palette[i].b));
        file1.commit();
        TextOutput file2( format("data-files/voxelTypes/vox%d.Any", i + numTexturedVoxels + 1) );
        file2.printf("{};");
		file2.commit();
    }
	
    for (const ParseVOX::Voxel& voxel : source.voxel){
        Point3int32 pos = Point3int32(voxel.position.x, voxel.position.z, voxel.position.y);
        setVoxel(pos, voxel.index + numTexturedVoxels);
    }
	
    voxTypeCount += palSize;

}

void App::addColorMaterials() {
    
    float colorSize = std::pow(rainbowSize, 1.0f / 3.0f);
    float stepSize = 1.0f / colorSize;
    int x = 0;
    int y = 0;
    int z = 0;
    for (int i = 0; i < rainbowSize; ++i){
        float temporary;
        temporary = float(i) / (colorSize * colorSize);
        x = int(temporary);
        temporary = (i - x * (colorSize * colorSize)) / colorSize;
        y = int(temporary);
        z = (i - x * (colorSize * colorSize) - y * colorSize);
        

        TextOutput file1(format("data-files/texture/color%d.UniversalMaterial.Any", i + 1));
        file1.printf("UniversalMaterial::Specification {lambertian = Color3(%f, %f, %f);}", x * stepSize, y * stepSize, z * stepSize);
        file1.commit();
        TextOutput file2(format("data-files/voxelTypes/vox%d.Any", i + voxTypeCount + 1));
        file2.commit();
    }
    voxTypeCount += rainbowSize;
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
    
	addColorMaterials();
    for (int i = 1; i < 33; ++i) {
         m_voxToMat.append(UniversalMaterial::create( Any::fromFile(format("data-files/texture/colorglass%d.UniversalMaterial.Any", i)) ));
    }

    for (int i = 0; i < palSize; ++i) {    
        m_voxToMat.append(UniversalMaterial::create( Any::fromFile(format("data-files/texture/pal%d.UniversalMaterial.Any", i)) ));
    }
	for (int i = 0; i < rainbowSize; ++i) {    
        m_voxToMat.append(UniversalMaterial::create( Any::fromFile(format("data-files/texture/color%d.UniversalMaterial.Any", i + 1)) ));
    }

    //This material is used for debug only, and stays at the end of voxToMat.
    m_voxToMat.append(UniversalMaterial::create( Any::fromFile("data-files/texture/glass/glass.UniversalMaterial.Any")));

}

void App::initializeSounds() {

	m_sounds.append(
		Sound::create("data-files/sounds/ui_wood_back.wav"),
		Sound::create("data-files/sounds/ui_wood_error.wav"),
		Sound::create("data-files/sounds/ui_wood_open.wav"),
		Sound::create("data-files/sounds/ui_casual_pops_back.wav"),
		Sound::create("data-files/sounds/ui_casual_pops_close.wav"),
		Sound::create("data-files/sounds/ui_casual_pops_confirm.wav")
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
    String modelName = model->name();

    if (mt.containsKey(modelName)){
        scene()->removeModel(modelName);
    }
    scene()->insert(model);

    // Replace any existing entity that has the wrong type
    shared_ptr<Entity> entity = scene()->entity(entityName);
    if (notNull(entity) && isNull(dynamic_pointer_cast<VisibleEntity>(entity))) {
        logPrintf("The scene contained an Entity named %s that was not a VisibleEntity\n", entityName);
        scene()->remove(entity);
        entity.reset();
    }

    if (isNull(entity)) {
        // There is no entity in this scene, so make one.
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

        anyFile["model"] = modelName;
        entity = scene()->createEntity(entityName, anyFile); //TODO

    } else {
        // Change the model on the existing entity
        dynamic_pointer_cast<VisibleEntity>(entity)->setModel(model);
    }

	//TODO
    shared_ptr<VisibleEntity> visibleEntity = scene()->typedEntity<VisibleEntity>(entityName);
    visibleEntity->setVisible(visible);
    return visibleEntity;
}

void App::removeEntity(shared_ptr<VisibleEntity> entity){
    scene()->removeEntity(entity->name());
}

// Returns true if the positions given is occupied.
bool App::voxIsSet(Point3int32 pos) {
    return getChunk(pos)->contains(pos);
}

/** Used explicitly for removing voxels, check whether the voxel is the boundary of a chunk.
	If it is, push the neighboring chunk to the m_chunksToRedraw for later update. 
	Can add up to 2 more chunks for update. */
void App::checkBoundaryAdd(Point3int32 pos){
    Point2int32 current = Chunk::getChunkCoords(pos);
    Point2int32 xPlus   = Chunk::getChunkCoords(pos + Point3int32(1,0,0));
    Point2int32 xMinus  = Chunk::getChunkCoords(pos + Point3int32(-1,0,0));
    Point2int32 zPlus   = Chunk::getChunkCoords(pos + Point3int32(0,0,1));
    Point2int32 zMinus  = Chunk::getChunkCoords(pos + Point3int32(0,0,-1));

    if ( (xPlus != current) ) {
        markChunkDirty(pos + Point3int32(1,0,0));
    } 
    if ( (xMinus != current) ) {
        markChunkDirty(pos+ Point3int32(-1,0,0));
    }
    if ( (zPlus != current) ) {
        markChunkDirty(pos + Point3int32(0,0,1));
    } 
    if ( (zMinus != current) ) {
        markChunkDirty(pos+ Point3int32(0,0,-1));
    }
}

/** Return the chunk a given voxel resides in. */
shared_ptr<Chunk> App::getChunk(Point3int32 pos) {
    Point2int32 key = Chunk::getChunkCoords(pos);
    if ( !m_posToChunk.containsKey(key) ) {
        m_posToChunk.set( key, std::make_shared<Chunk>(Chunk(key)) );
    }
    return m_posToChunk[key];
}

/** Return the voxel type at a given grid position. */
int App::posToVox(Point3int32 pos) {
    return getChunk(pos)->get(pos);
}

/** Set the voxel at a given grid position in the world data structure. */
void App::setVoxel(Point3int32 pos, int type) {
    shared_ptr<Chunk> chunk = getChunk(pos);
	if ( !voxIsSet(pos) ) {
		chunk->set(pos, type);
	}
}

/** Unset the voxel at a given grid position in the world data structure. */
void App::unsetVoxel(Point3int32 pos) {
    shared_ptr<Chunk> chunk = getChunk(pos);
	if ( voxIsSet(pos) ) {
		chunk->remove(pos);
	}
    markChunkDirty(pos);
    checkBoundaryAdd(pos);
}

/** Marks a chunk for updating */
void App::markChunkDirty(Point3int32 pos) {
    Point2int32 chunkPos = Chunk::getChunkCoords(pos);
    if(m_chunksToUpdate.findIndex(chunkPos) == -1) {
        m_chunksToUpdate.push(chunkPos);
    }
}


void App::clearChunkGeometry(Point2int32 chunkPos) {
    for (int i = 0; i < voxTypeCount; ++i) {
	    if ( notNull(m_model->geometry(format("geom %d,%d,%d", chunkPos.x, chunkPos.y, i))) ) {
            ArticulatedModel::Geometry* geometry = m_model->geometry(format("geom %d,%d,%d", chunkPos.x, chunkPos.y, i ));
            ArticulatedModel::Mesh*     mesh     = m_model->mesh(format("mesh %d,%d,%d", chunkPos.x, chunkPos.y, i ));

	        Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
	        Array<int>&					   indexArray  = mesh->cpuIndexArray;

            vertexArray.fastClear();
	        indexArray.fastClear();

			// Clean geometry
            ArticulatedModel::CleanGeometrySettings geometrySettings;
            geometrySettings.allowVertexMerging = false;
            m_model->cleanGeometry(geometrySettings);
            geometry->clearAttributeArrays();
            mesh->clearIndexStream();
        
            bool hasSomething = false;
            for (Table<Point3int32, int>::Iterator it = m_posToChunk[chunkPos]->begin(); it.isValid(); ++it) {
                 if ((*it).value == i) {
                     hasSomething = true;
                 } 
            }

			if ( !hasSomething ) {
			    for (int i = 0; i < 3; ++i) {
			        CPUVertexArray::Vertex& dummy = vertexArray.next();
			        dummy.position = Point3(0, 0, 0);
			        dummy.texCoord0 = Point2(0, 0);
			        dummy.normal  = Vector3::nan();
			        dummy.tangent = Vector4::nan();
			    }
			    
				indexArray.append(0,1,2);
			}
        }  
    }
}

void App::createChunkGeometry(Point2int32 chunkPos) {
	
    for (Table<Point3int32, int>::Iterator it = m_posToChunk[chunkPos]->begin(); it.hasMore(); ++it) {
	    createVoxelGeometry( it->key );
	}
    
	// Update the geometry for every type
    for (int i = 0; i < voxTypeCount; ++i) { 
        cleanChunkGeometry(chunkPos, i);
    }
    scene()->entity("voxel")->markChanged();
}

// Clean/update the geometry for our model.
void App::cleanChunkGeometry(Point2int32 chunkCoords, int type) {
    
    if ( notNull(m_model->geometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type))) ) {
		ArticulatedModel::Geometry* geometry = m_model->geometry(format("geom %d,%d,%d", chunkCoords.x, chunkCoords.y, type));
        ArticulatedModel::Mesh*     mesh	 = m_model->mesh(format("mesh %d,%d,%d", chunkCoords.x, chunkCoords.y, type));

	    ArticulatedModel::CleanGeometrySettings geometrySettings;
        geometrySettings.allowVertexMerging = false;
        m_model->cleanGeometry(geometrySettings);

	    // If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
	    geometry->clearAttributeArrays();

	    // If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
    	mesh->clearIndexStream();

	}

}

void App::updateChunks() {
    for (int i = 0; i < m_chunksToUpdate.size(); ++i) {        
        clearChunkGeometry(m_chunksToUpdate[i]);
        createChunkGeometry(m_chunksToUpdate[i]);
    }
    m_chunksToUpdate.fastClear();
}

//Redraw the geometry for the entire voxel world.
void App::redrawWorld() {
    Array<Point2int32> chunkArray = m_posToChunk.getKeys();
    for (int i = 0; i < chunkArray.size(); ++i) {
        clearChunkGeometry(chunkArray[i]);
        createChunkGeometry(chunkArray[i]);
    }
}


// Input = Center of vox
void App::addVoxel(Point3int32 input, int type) {
    setVoxel(input, type);

    createVoxelGeometry(input);
    cleanChunkGeometry(Chunk::getChunkCoords(input), type);
	
	SoundIndex i = add;
	m_sounds[i]->play();
}

void App::createVoxelGeometry(Point3int32 input) {
    int type = posToVox(input);
	ArticulatedModel::Geometry* geometry;
	ArticulatedModel::Mesh*     mesh;

	Point2int32 chunkCoords = Chunk::getChunkCoords(input);
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
void App::createNaiveVoxelGeometry(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, int type) {
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

void App::removeVoxel(Point3int32 input) {
	unsetVoxel(input);

	SoundIndex i = remove;
	m_sounds[i]->play();
}


void App::cameraIntersectVoxel(Point3int32& lastPos, Point3int32& hitPos){
    
	// Intersect with empty space
    const float maxDist = 2.0f + m_intersectMode * 2.0f;
    Vector3 direction = m_crosshair.lookDirection;
    
    Ray cameraRay(m_crosshair.position, m_crosshair.lookDirection);

    //Point2 center = UserInput(this->window()).mouseXY();
    //Ray cameraRay = activeCamera()->worldRay(center.x / this->window()->width() * renderDevice->width(), center.y / this->window()->height() * renderDevice->height(), renderDevice->viewport());

    hitPos = Point3int32( (m_crosshair.position + m_crosshair.lookDirection * maxDist) / voxelRes );

    lastPos = hitPos;

	// Intersect with voxel
    if (!forceIntersect) {
		// the boundary of the voxels that would intersect
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

void App::drawSelectionPreview(){

	addEntity(m_debugModel, "debugEntity");
	ArticulatedModel::Part* part = m_debugModel->addPart("root");
	int type = voxTypeCount;
	
	ArticulatedModel::Geometry* geometry;
	ArticulatedModel::Mesh*     mesh;
	
	if ( isNull(m_debugModel->geometry("geom")) )  {
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

	for (int i = 0; i < selectionArray.length(); ++i) {
	    Point3int32 pos = selectionArray[i];
		if (voxIsSet(pos)) {
			Point3 selection = Util::voxelToWorldSpace(pos);
			createNaiveVoxelGeometry(geometry, mesh, selection, voxelRes * 1.1f, type);
		}
	}
	
	if (selectionArray.length() == 0) {
		createNaiveVoxelGeometry(geometry, mesh, Point3(0,0,0), 0, type);
	}
	
	ArticulatedModel::CleanGeometrySettings geometrySettings;
	geometrySettings.allowVertexMerging = false;
	m_debugModel->cleanGeometry(geometrySettings);
	
	// If you modify cpuVertexArray, invoke this method to force the GPU arrays to update
	geometry->clearAttributeArrays();
	
	// If you modify cpuIndexArray, invoke this method to force the GPU arrays to update on the next ArticulatedMode::pose()
	mesh->clearIndexStream();
}

void App::selectSphere(Point3 origin, Vector3 direction) {
	m_selection.setMode(m_selectionMode);
    m_selection.commitSphere(origin, direction);
    drawSelectionPreview();
}

void App::selectCylinder(Point3int32 center, int radius) {
	m_selection.setMode(m_selectionMode);
	m_selection.commitCylinder(center, radius);
    drawSelectionPreview();
}

void App::selectBox(Point3int32 center, int radius) {
	m_selection.setMode(m_selectionMode);
	m_selection.commitBox(center, radius);
    drawSelectionPreview();
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

        for (int i = 0; i < changeBuffer.size(); ++i) {
            Point3int32 targetPos = changeBuffer[i] + Point3int32(0, delta, 0);
            setVoxel(targetPos, posToVox(changeBuffer[i]));
         
            unsetVoxel(changeBuffer[i]);
        }
    }
    m_selection.clear();
    drawSelectionPreview();

	SoundIndex i = elevate;
	m_sounds[i]->play();
}


void App::makeCrater(Point3int32 center, int radius) {

	// Lambda function for AnimationControl.
	// sdt = differential (time since last call of this function)
	// st = absolute (time since animation began)
	std::function<void (SimTime, SimTime, shared_ptr<Table<String, float>>, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&)> lambda = [&](SimTime sdt, SimTime st, shared_ptr<Table<String, float>> args, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&) {

		int currentRadius = (int)(args->get("currentRadius"));
		int radius = (int)args->get("radius");
		Point3int32 center(args->get("centerX"), args->get("centerY"), args->get("centerZ"));

		SimTime threshold = 0.005f * currentRadius;

		if ( currentRadius < radius ) {
			if ( st > threshold ) {
				float bound = max(currentRadius / sqrt(2.0f) - 1.0f, 0.0f);
				float depth = (float)sqrt(radius - currentRadius);
				
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

void App::setUpCurrentTransform(){
    Point3int32 lastPos, hitPos;
    cameraIntersectVoxel(lastPos, hitPos);
    if (currentTransform == "shockwave") {
        Vector3 direction = Vector3(m_crosshair.lookDirection.x, 0.0f, m_crosshair.lookDirection.z);
        makeShockWaveArc(hitPos, direction);
    } else if (currentTransform == "orbit") {
		if(m_hasOrbit){
			m_throwStartPos = m_hand2->frame().translation;
		}
	} else {
		m_currentMark = hitPos;
    }

}

void App::commitCurrentTransform(){
    Point3int32 lastPos, hitPos;
    cameraIntersectVoxel(lastPos, hitPos);
    if (currentTransform == "mountain") {
		makeMountain(m_currentMark, hitPos.y - m_currentMark.y);
    } else if (currentTransform == "elevate") {
		elevateSelection(hitPos.y - m_currentMark.y);
	} else if (currentTransform == "crater") {
		makeCrater(m_currentMark, Vector3(hitPos.x - m_currentMark.x, hitPos.y - m_currentMark.y, hitPos.z - m_currentMark.z).magnitude());
	}else if(currentTransform == "orbit"){
		if(m_hasOrbit){
			Vector3 throwDirection = m_hand2->frame().translation - m_throwStartPos;
			flingSatellite(throwDirection, 2);
		}else if( (m_hand1->frame().translation - m_hand2->frame().translation).length() > 0.25){
			pullVoxelOrbit(hitPos);
		}
	
	}

}
void App::makeShockWaveArc(Point3int32 origin, Vector3 direction){
	debugPrintf("ARC ON!!!\n");
	Point3int32 testPos;
	Vector3 testDirection;
	for (int i = -4; i < 4; ++i){
		int x = origin.x + i;
		for(int j = -4; j < 4; ++j){
			int z = origin.z + j;
			testPos.x = x;
			testPos.y = origin.y;
			testPos.z = z;

			testDirection = origin - testPos;

			if(abs((float)aCos(testDirection.dot(direction))) < PI/4.0f &&
				(testDirection.length() > 5 && testDirection.length() < 10)){

				makeShockWave(origin, direction);
			}
			 
		}
	
	
	}

}

void App::makeShockWave(Point3 origin, Vector3 direction) {
    debugPrintf("SHOCKWAVE START\n");
    std::function<void (SimTime, SimTime, shared_ptr<Table<String, float>>, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&)> lambda = [&](SimTime sdt, SimTime st, shared_ptr<Table<String, float>> args, Table<Point3int32, shared_ptr<VisibleEntity>>& entities, Table<Point3int32, SimTime>& timeAdded) {

        float bounds = (st / 12.0f) * (float)(1 << 7); //Bounds should increase over time

        //the Boundary of the voxels that would intersect
		Point3int32 voxelBound = Point3int32( (int)bounds, (int)bounds, (int)bounds );

        Point3int32 origin = Point3int32(args->get("originX"), args->get("originY"), args->get("originZ"));
		Vector3 direction = Vector3(args->get("directionX"), 0, args->get("directionZ"));

        Point3int32 rayOrigin(0, -1, 0);
        debugPrintf("%d\n", rayOrigin.y);
        Point3int32 diff = origin - rayOrigin;

        Ray shockWaveRay(rayOrigin, direction);

        int i = 0;
        int numVoxels = 500;
		Random rand;
		for (RayGridIterator it(shockWaveRay, voxelBound, Vector3(voxelRes,voxelRes,voxelRes), Point3(-voxelBound / 2) * voxelRes, -voxelBound / 2); it.insideGrid() && i < numVoxels; ++it) {

            Point3int32 voxCoord = it.index() + diff;
            Point3int32 centerCoord = voxCoord;
            voxCoord.y += 1;

			if ( voxIsSet(voxCoord) ) {						
				//debugPrintf("%d\n", voxCoord.y);
				shared_ptr<VisibleEntity> ent;
				SimTime timePassed;
				String name = format("animatedvox%d,%d,%d", voxCoord.x, voxCoord.y, voxCoord.z);
				
				//We will check to see if our voxels are already in our entity list, if not we add them and 
				//note the time they were added.
				if ( !entities.containsKey(voxCoord) ) {
				    shared_ptr<ArticulatedModel> model = makeVoxelModel( name, posToVox( voxCoord ) );    
				    ent = addEntity(model, name, true);
				    entities.set(voxCoord, ent);
				    timeAdded.set(voxCoord, st);
				    timePassed = 0.0f;
				} else {
				    ent = entities[voxCoord];
				    timePassed = (st - timeAdded[voxCoord]);
				
				}

				// The rise that we are going to add to these entities is depenedent on the time added. They should
				// go up and down similar to a sine curve.
				float rise;
				float totalTime = 0.25f;
				float halfTime = totalTime/2.0f;
				float maxRise = 0.7f;
				float currentTime = st;
				currentTime;
				if (timePassed <= halfTime) {
				    rise = (timePassed/(halfTime))*maxRise;
				} else if (timePassed <= totalTime) {
				    rise = ( ( (halfTime) - (timePassed - halfTime ) ) / (halfTime) ) * maxRise;
				} else {
				    ent->setVisible(false);
				
				    if ( notNull(scene()->entity(ent->name())) ) {
				        removeEntity(ent);
				    }
				}

				Point3int32 voxPos = Point3int32(voxCoord.x, voxCoord.y, voxCoord.z);
				Point3 worldPos = Util::voxelToWorldSpace(voxPos);
				CoordinateFrame frame = CFrame::fromXYZYPRRadians(worldPos.x, worldPos.y + rise, worldPos.z);
				ent->setFrame(frame);
				++i;

	        } else {
				break;
            }
        }

        lastAnimFinished = st > 10.0f;
        //printf("finishFrame\n");
		
        if ( lastAnimFinished ) {
            Array<shared_ptr<VisibleEntity>> ents;
            entities.getValues(ents);
            for(int i = 0; i < ents.size(); ++i){
                if (notNull(scene()->entity(ents[i]->name()))) {
                    removeEntity(ents[i]);   
                }
            }
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

void App::makeMountain(Point3int32 center, int height) {
	
	std::function<void (SimTime, SimTime, shared_ptr<Table<String, float>>, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&)> lambda = [&](SimTime sdt, SimTime st, shared_ptr<Table<String, float>> args, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&) {

		int currentRadius = (int)(args->get("currentRadius"));
		int radius = (int)args->get("radius");
		int currentHeight = (int)(args->get("currentHeight"));
		int height = (int)args->get("height");
		Point3int32 center(args->get("centerX"), args->get("centerY"), args->get("centerZ"));

		SimTime threshold = 0.005f * currentRadius;

		if ( currentHeight < height ) {
			if ( st > threshold ) {
				Noise n = Noise::common();

				for (Point3int32 P(center.x - currentRadius, center.y + currentHeight, center.z); P.x <= center.x + currentRadius; ++P.x) {
					for (P.z = center.z - currentRadius; P.z <= center.z + currentRadius; ++P.z) {
						
						float noise = n.sampleFloat(P.x, P.y, P.z);

						Point3int32 pos(P.x, P.y + noise, P.z);

						if ( sqrt((pos.x - center.x) * (pos.x - center.x) + (pos.z - center.z) * (pos.z - center.z)) <= currentRadius && !voxIsSet(pos) ) {
							setVoxel(pos, m_voxelType);
							
							markChunkDirty(pos);
						}
					
					}
				}

				int a = Random::threadCommon().integer(0, 1);
				if (a == 1) {
					currentHeight++;
					args->set("currentHeight", float(currentHeight));
				}
				int b = Random::threadCommon().integer(0, 1);
				if (b == 1) {
					currentRadius--;
					args->set("currentRadius", float(currentRadius));
				}
			}
		}

		if ( (currentHeight == height) || (currentRadius == -1) ) {
			lastAnimFinished = true;
		} else {
			lastAnimFinished = false;
		}

	};


	AnimationControl a(lambda);
	a.args->set("currentRadius", float(height));
	a.args->set("radius", float(height));
	a.args->set("currentHeight", 1.0f);
	a.args->set("height", float(height));
	a.args->set("centerX", float(center.x));
	a.args->set("centerY", float(center.y));
	a.args->set("centerZ", float(center.z));
	m_animControls.push(a);

}

/** Method to pull voxels away from the world and make them orbit the player */
void App::pullVoxelOrbit(Point3int32 origin) {

    m_hasOrbit = true;

    Table<Point3int32, shared_ptr<VisibleEntity>> entities;
    
    //Build a buffer of voxels around the origin
    const int maxOrbitSize = 12;
    Array<Point3int32> queue, buffer;
    queue.push(origin);
    while(buffer.size() < maxOrbitSize && queue.size() > 0) {
        Point3int32 current = queue.first();
        queue.fastRemove(0);
        for(Point3int32 P(-1, -1, -1); P.x <= 1; ++P.x) {
            for(P.y = -1; P.y <= 1; ++P.y) {
                for(P.z = -1; P.z <= 1; ++P.z) {
                    if(voxIsSet(current + P) && buffer.findIndex(current + P) == -1) {

                        //Add the animated entity to the scene
                        String name = format("orbitingVox%d", buffer.size());
                        shared_ptr<ArticulatedModel> model = makeVoxelModel( name, posToVox( current + P ) );    
                        shared_ptr<VisibleEntity> ent = addEntity(model, name, true);
                        entities.set(Point3int32(buffer.size(), 0, 0), ent);

                        //Put the entity at the correct position
                        Point3 worldPos = Util::voxelToWorldSpace(current + P);
                        CoordinateFrame frame = CFrame::fromXYZYPRRadians(worldPos.x, worldPos.y, worldPos.z);
                        ent->setFrame(frame);

                        //Remove the voxel from the world
                        buffer.push(current + P);
                        unsetVoxel(current + P);
                        queue.push(origin);
                    }
                }
            }
        }
    }


    std::function<void (SimTime, SimTime, shared_ptr<Table<String, float>>, Table<Point3int32, shared_ptr<VisibleEntity>>&, Table<Point3int32, SimTime>&)> lambda = [&](SimTime sdt, SimTime st, shared_ptr<Table<String, float>> args, Table<Point3int32, shared_ptr<VisibleEntity>>& activeVoxels, Table<Point3int32, SimTime>& timeAdded) {

        //Constant to vary how fast the entities are picked up.
        const float startDelay = 0.5f;

        //Constant to vary how fast the entities should travel.
        const float speed = 8.0f;

        //Constant to vary how large the initial offset is.
        const float offsetSize = 5.0f;

        //Constant to vary the orbit radius.
        const float orbitRadius = 2.0f;

        //Get arguments.
        Vector3 normal(args->get("normalX"), args->get("normalY"), args->get("normalZ"));
        int numSatellites = args->get("numSatellites");
        if(args->containsKey("fling")) {
            Vector3 flingDir(args->get("flingX"), args->get("flingY"), args->get("flingZ"));
            Point3 flingTarget = flingDir + activeCamera()->frame().translation;
            int satToFling = -1;
            Point3 closestSatPos;

            //Choose which satellite to fling
            for(Table<Point3int32, shared_ptr<VisibleEntity>>::Iterator it = activeVoxels.begin(); it.hasMore(); ++it) {
                if(satToFling == -1) {
                    satToFling = it->key.x;
                    closestSatPos = it->value->frame().translation;
                } else {
                    Point3 challengerPos = it->value->frame().translation;
                    if((challengerPos - flingTarget).magnitude() < (closestSatPos - flingTarget).magnitude()) {
                        satToFling = it->key.x;
                        closestSatPos = challengerPos;
                    }
                }
            }

            args->remove("fling");
            args->remove("flingX");
            args->remove("flingY");
            args->remove("flingZ");

            args->set(format("fling%dX", satToFling), flingDir.x);
            args->set(format("fling%dY", satToFling), flingDir.y);
            args->set(format("fling%dZ", satToFling), flingDir.z);
            numSatellites--;
            args->set("numSatellites", numSatellites);
        }

        //Loop over all satellites.
        for(Table<Point3int32, shared_ptr<VisibleEntity>>::Iterator it = activeVoxels.begin(); it.hasMore(); ++it) {
            int satNum = it->key.x;

            if(args->containsKey(format("fling%dX", satNum))) {
                
                Vector3 flingDir(args->get(format("fling%dX", satNum)), args->get(format("fling%dY", satNum)), args->get(format("fling%dZ", satNum)));
                Point3 start = it->value->frame().translation;
                if((start - activeCamera()->frame().translation).magnitude() > 200.0f) {
                    removeEntity(it->value);
                    activeVoxels.remove(it->key);
                } else {
                    it->value->setFrame(CoordinateFrame(start + flingDir));
                }
            } else {

                //Check to make sure the entity has started moving.
                if(st > sqrt(satNum) * startDelay ) {

                    //Setup the coordinate frame to get the offset vector.
                    CoordinateFrame normalFrame;
                    normalFrame.lookAt(normal);
                    Vector3 offset(offsetSize * cos(2 * PI / numSatellites * satNum), 0.0f, offsetSize * sin(2 * PI / numSatellites * satNum));
                    offset = normalFrame.pointToWorldSpace(offset);

                    //Setup the target point for the given satellite.
                    Vector3 target(orbitRadius * cos( speed * st / orbitRadius + 2 * PI / numSatellites * satNum), 0.0f, orbitRadius * sin(speed * st / orbitRadius + 2 * PI / numSatellites * satNum));
                    target = CoordinateFrame(this->activeCamera()->frame().translation).pointToWorldSpace(target);
                    
                    //Set the new frame for the entity.
                    Point3 start = it->value->frame().translation;
                    Point3 end = start + (speed * (target - start + (offset / (st - sqrt(satNum) * startDelay))).direction() * sdt);
                    
                    CoordinateFrame endFrame = CoordinateFrame(end);
                    it->value->setFrame(endFrame);
                }
            }
        }
    
        if(numSatellites == 0) {
            for(Table<Point3int32, shared_ptr<VisibleEntity>>::Iterator it = activeVoxels.begin(); it.hasMore(); ++it) {
                removeEntity(it->value);
            }
            lastAnimFinished = true;
            m_hasOrbit = false;
        } else {
            lastAnimFinished = false;
        }
    };

    AnimationControl a(lambda);
    a.activeVoxels = entities;
    Vector3 normal = activeCamera()->frame().translation - origin;
    a.args->set("orbit", 1.0f);
    a.args->set("normalX", normal.x);
    a.args->set("normalY", normal.y);
    a.args->set("normalZ", normal.z);
    a.args->set("numSatellites", buffer.size());

    m_animControls.push(a);

}

void App::flingSatellite(Vector3 dir, float speed) {
    dir = dir.direction();
    dir *= speed;
    for(int i = 0; i < m_animControls.size(); ++i) {
        shared_ptr<Table<String, float>> args = m_animControls[i].args;
        if(args->containsKey("orbit")) {
            args->set("fling", 1.0f);
            args->set("flingX", dir.x);
            args->set("flingY", dir.y);
            args->set("flingZ", dir.z);
        }
    }
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

    // Change intersect distance
    else if ( (event.type == GEventType::MOUSE_SCROLL_2D) ){ 
		m_intersectMode += event.scroll2d.dy;
    }

    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('m')) ){ 
		menuMode = !menuMode;
		SoundIndex i;
		
		if (menuMode) {
		    menuFrame = activeCamera()->frame();
		    m_menu[m_currentMenuPage]->setFrame(menuFrame);
		
			i = menuOpen;
		} else {
			i = menuClose;
		}
		
		m_sounds[i]->play();
		
		m_menu[m_currentMenuPage]->setVisible(menuMode);
    }

    // Change voxel types
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('0')) ){ 
		m_voxelType = 0;
    }
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('1')) ){ 
		m_voxelType = 1;
    }
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('2')) ){ 
		m_voxelType = 2;
    }
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('3')) ){ 
		m_voxelType = 3;
    }
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('4')) ){ 
		m_voxelType = 4;
    }
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('5')) ){ 
		m_voxelType = 5;
    }
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('6')) ){ 
		m_voxelType = 6;
    }
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('7')) ){ 
		m_voxelType = 7;
    }
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('8')) ){ 
		m_voxelType = 8;
    }

    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('t')) ){
		if ( m_currentMenuPage == 0 ) {
			changeMenuPage( numMenuPages - 1 );
		} else {
			changeMenuPage( m_currentMenuPage - 1);
		}
    }

    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('y')) ){ 
		changeMenuPage((m_currentMenuPage + 1) % numMenuPages);
    }
	
	// Shock wave
	else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('o')) ) { 
        Point3int32 hitPos;
        Point3int32 lastPos;
        m_crosshair.lookDirection = Vector3(0,-1,0);
        cameraIntersectVoxel(lastPos, hitPos);
        updateCrosshair();
        Vector3 direction = Vector3(m_crosshair.lookDirection.x, 0.0f, m_crosshair.lookDirection.z);

        if(lastPos != hitPos){
            makeShockWaveArc(hitPos, direction);
        }
    }

    // Force intersect
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('f')) ){ 
		forceIntersect = !forceIntersect;
    }

    // Change selection mode
    else if ( (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey('n')) ){ 
		m_selectionMode %= 4;
		m_selectionMode++;
		debugPrintf("mode: %d\n", m_selectionMode);
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
		makeMountain(m_currentMark, hitPos.y - m_currentMark.y);
    }
    //Orbit
    else if ( (event.type == GEventType::KEY_UP) && (event.key.keysym.sym == GKey('b')) ) {
        Point3int32 hitPos;
        Point3int32 lastPos;
        cameraIntersectVoxel(lastPos, hitPos);
        if(!m_hasOrbit) {
		    pullVoxelOrbit(hitPos);
        } else {
            flingSatellite(m_crosshair.lookDirection, 5.0f);
        }
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

    // Handle super-class events

    return false;
}

void App::onUserInput(UserInput* ui) {
    
    super::onUserInput(ui);

    if ( !vrEnabled ) {
        updateCrosshair();
    }
    
}

void App::onGraphics(RenderDevice * rd, Array< shared_ptr< Surface > > & surface, Array< shared_ptr< Surface2D > > & surface2D ) {

    updateChunks();
    if (menuMode) {
        CFrame frame = menuFrame;
        if (menuType == 1 ) {
			GuiText menuText = format("Select Transform Type");
			debugDrawLabel(frame.pointToWorldSpace(m_menuButtonPositions[menuPageSize]), Vector3(0,0,0), menuText);
			for (int i = 0; i < m_transforms.size(); ++i) {
			    menuText = m_transforms[i];
			    debugDrawLabel(frame.pointToWorldSpace(m_menuButtonPositions[i]), Vector3(0,0,0), menuText);
			}
        } else {
			GuiText menuText = format("Select Voxel Type (Pg. %d/%d)", m_currentMenuPage + 1, numMenuPages); //TODO
			debugDrawLabel(frame.pointToWorldSpace(m_menuButtonPositions[menuPageSize]), Vector3(0,0,0), menuText);
        }
    }
    if (vrEnabled) {
        //updateCrosshair();
        //Point3 head;
        //Point3 hand1;
        //Point3 hand2;
        //if(m_vrControllerArray.size() > 0 && notNull(m_hand1)){
        //    //hand1 = m_vrControllerArray[0]->frame().pointToWorldSpace(Point3(0,0,0));
        //    m_hand1->setFrame(m_vrControllerArray[0]->frame());
        //}
        //if(m_vrControllerArray.size() > 1 && notNull(m_hand2)){
        //   // hand2 = m_vrControllerArray[1]->frame().pointToWorldSpace(Point3(0,0,0));
        //    m_hand2->setFrame(m_vrControllerArray[1]->frame());
        //}
		//
		//
		////CurrentTransform indicator
		//CFrame player = CoordinateFrame(m_vrHead->frame().translation);
		//GuiText text = currentTransform;
		//debugDrawLabel(player.pointToWorldSpace(Point3(0,-1,0)), Vector3(0,0,0), text);
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
        //
        //vr::VREvent_t vrEvent;
        //while (m_hmd->PollNextEvent(&vrEvent, sizeof(vrEvent))) {
		//	bool mainHand = (vrEvent.trackedDeviceIndex - 1) % 2 == 0;
		//	switch (vrEvent.eventType) {
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
        //                cameraIntersectVoxel(lastPos, hitPos);
        //                if(mainHand){        
        //                    addVoxel(lastPos, m_voxelType);
        //                }else{
        //                    removeVoxel(hitPos);
        //                }
        //                break;
        //            
        //            //Grip
        //            case 2:
        //                if(mainHand){
		//					cameraIntersectVoxel(lastPos, hitPos);
		//					m_selection.setMark(hitPos);               
		//
		//				}else{
		//					setUpCurrentTransform();
		//				}
        //                break;
        //            
        //            //Touchpad Click
        //            case 32:
        //              
        //                cameraIntersectVoxel(lastPos, hitPos);
        //                m_debugCamera->setFrame(CFrame(Util::voxelToWorldSpace(lastPos)));
        //                m_debugController->setFrame(m_debugCamera->frame());
        //                updateCrosshair();
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
        //                    if(menuType == 0){
        //                        m_voxelType = m_crosshair.voxelIndex;
        //                        updateHands(m_crosshair.voxelIndex);
        //                    }else{
        //                        currentTransform = m_transforms[m_crosshair.buttonIndex];
        //                    }
        //                  
        //                
        //                }
        //                break;
        //            case 2:
        //                switch((vrEvent.trackedDeviceIndex - 1) % 2){
        //                case 1:
        //                    if ( m_currentMenuPage == 0 ) {
		//                    	changeMenuPage( numMenuPages - 1 );
		//                    } else {
		//                    	changeMenuPage( m_currentMenuPage - 1 );
		//                    }
        //                    break;
        //                default:
        //                    changeMenuPage((m_currentMenuPage + 1) % numMenuPages);
        //                    break;
        //                
        //                }
        //            
        //            default:
        //                debugPrintf("INPUT NOT RECOGNIZED\n");
        //                break;
        //            }
        //        }
        //
        //       if(vrEvent.data.controller.button == 1){
        //            menuType = (vrEvent.trackedDeviceIndex - 1) % 2;
        //            menuMode = !menuMode;
        //            if(menuMode){
        //                m_crosshair.menuControllerIndex = (vrEvent.trackedDeviceIndex - 1) % 2;
        //                setMenuFrame();
		//
        //          }
        //         if(menuType == 0){ 
        //              m_menu[m_currentMenuPage]->setVisible(menuMode);
        //         }
        //     }
        //
        //     break;
        //
        //  case vr::VREvent_ButtonUnpress:
		//		if(!menuMode){
		//			switch(vrEvent.data.controller.button){
		//			//grip	
		//			case 2:
		//				if(mainHand){
		//					selectSphere(m_crosshair.position, m_crosshair.lookDirection);
		//				}else{
		//					commitCurrentTransform();
		//				}
		//			}
		//		}
		//		
		//		break;
        //
        //    default:
        //        // Ignore event
        //        ;
        //    }
        //}
	}
	
	super::onGraphics(rd, surface, surface2D);
}