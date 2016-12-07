/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include <GLG3DVR/VRApp.h>
#include "Selection.h"
#include "AnimationControl.h"
#include "Chunk.h"
#include "Util.h"

class CrosshairObject{
public:
    Vector3 lookDirection;
    Point3 position;
    Ray ray;
    bool buttonSelected = false;
    int voxelIndex = 0;
    int buttonIndex = 0;
    int menuControllerIndex = 0;
};

typedef 
    //VRApp 
    GApp
    AppBase; 

class App : public AppBase {

protected:

    typedef AppBase super;

	enum VoxelType { grass, rock, brick, sand, cedar, metal, chrome, rubber };
	enum SoundIndex { add, remove, elevate, menuOpen, menuClose, menuSelect };

    const float voxelRes = 0.5f;
	const int menuPageSize = 12;
	int numMenuPages;
    /** Number of naive colors in rainbow voxels */
    const int rainbowSize = 100;

    int voxTypeCount = 8;

    int menuType = 0; // 0 is VoxelMenu 1 is TransformMenu


    Array<String> m_transforms;

    String currentTransform = "shockwave";


	bool lastAnimFinished = false;

    bool vrEnabled = false;

	// Options

	bool menuMode = false;
    bool forceIntersect = false;
	/** Distance of intersection */
    int m_intersectMode = 1;
	/** Active selection mode (0 = idle, 1 = regular, 2 = set union, 3 = set minus, 4 = set intersection */
	int m_selectionMode = 1;
	int m_currentMenuPage = 0;
	/** Active voxel type */
    int m_voxelType = 0;

	// Data structures
	/** Maps type of voxel to Any files containing its specific properties */
	Array<Any> m_voxToProp;

	/** Maps type of voxel to Material of that voxel */
	Array<shared_ptr<UniversalMaterial>> m_voxToMat;

    /** Maps 3D chunk positions to the Tables representing each individual chunk */
    Table<Point2int32, shared_ptr<Chunk>> m_posToChunk;

    /** Contains the chunk positions of the chunks that need to be updated */
    Array<Point2int32> m_chunksToUpdate;

	/** Non-VR GUI list of first 8 voxel types (not including rainbow voxels) */
	Array<String> m_typesList;

	Array<AnimationControl> m_animControls;

	Array<shared_ptr<Sound>> m_sounds;

	// Objects in the world
    /** Stores the scene model */
    const shared_ptr<ArticulatedModel>& m_model = ArticulatedModel::createEmpty("voxelModel");

    const shared_ptr<ArticulatedModel>& m_handModel = ArticulatedModel::createEmpty("handModel");
    shared_ptr<VisibleEntity> m_hand1;
    shared_ptr<VisibleEntity> m_hand2;



	/** Marks where the user is currently pointing */
    CrosshairObject m_crosshair;

    /** Stores the current voxel selection */
    Selection m_selection;

    /** Stores marked position when mid transform */
    Point3int32 m_currentMark;

    /** Drawing the debug voxels */
    const shared_ptr<ArticulatedModel>& m_debugModel = ArticulatedModel::createEmpty("debugModel");

    CFrame menuFrame;
    Array<Point3> m_menuButtonPositions;
    Array<shared_ptr<ArticulatedModel>> m_menuModels;
    Array<shared_ptr<VisibleEntity>> m_menu;

    /** Camera manipulator */
    shared_ptr<FirstPersonManipulator> m_cameraManipulator;

	// Menu functions
	void makeMenuPageModels();
	void makeMenuPageEntities();
    void setMenuFrame();

	// Initialization functions
    /** Called from onInit */
    void makeGUI();
    void initializeScene();
	void initializeMaterials();
	void initializeSounds();
	void initializeModel();

    void initializeHands();
    void updateHands(int index);

	/** Generates voxel types that are solid colors. */
    void addColorMaterials();

	const shared_ptr<ArticulatedModel> makeVoxelModel(String modelName, int type, float size = 0.5f);
    shared_ptr<VisibleEntity> addEntity(shared_ptr<ArticulatedModel> model, String entityName, bool visible=true);
    void removeEntity(shared_ptr<VisibleEntity> entity);

	// Menu functions
    void getMenuPositions();
	void changeMenuPage(int to);

	// Manipulating the Chunk data structure
    int posToVox(Point3int32 pos);

    /** Main functions for manipulating the data of the world */
	void setVoxel(Point3int32 pos, int type);
    void unsetVoxel(Point3int32 pos);
    bool voxIsSet(Point3int32 pos);
    shared_ptr<Chunk> getChunk(Point3int32 pos);

	/** Create geometry for a single voxel */
    void createVoxelGeometry(Point3int32 input);

    /** Create every face for a voxel */
	void createNaiveVoxelGeometry(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, int type);

    /** Create the geometry for a single face */
	void addFace(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, Vector3 normal, Vector3::Axis axis, int type);
	
    /** Calls setVoxel and createVoxelGeometry */
    void addVoxel(Point3int32 input, int type);

    /** Calls unsetVoxel and marks the chunk as needing an update */
	void removeVoxel(Point3int32 input);

	/** Called from updateChunks() */
    void clearChunkGeometry(Point2int32 chunkPos);
	/** Called from updateChunks() */
    void createChunkGeometry(Point2int32 chunkPos);
	/** Called from updateChunks() */
    void cleanChunkGeometry(Point2int32 chunkCoords, int type);

	/** Used explicitly for removing voxels, check whether the voxel is the boundary of a chunk.
		If it is, push the neighboring chunk to the m_chunksToRedraw for later update. 
		Can add up to 2 more chunks for update. */
	void checkBoundaryAdd(Point3int32 pos);

	/** Visualize the user's currently selected voxels. */
    void drawSelectionPreview();

	// Intersecting scene and drawing crosshair
    void cameraIntersectVoxel(Point3int32& lastOpen, Point3int32& voxelTest);
    Ray getVrCrosshairRay();
    Ray getMouseCrosshairRay();
    void updateCrosshair();
    void updateMenuCrosshair();
    void setCrosshair(Ray crosshairRay);
    void drawCrosshair();

public:

    App(const super::Settings& settings = super::Settings());
    virtual void onInit() override;
    virtual bool onEvent(const GEvent& event) override;
    virtual void onUserInput(UserInput* ui) override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onGraphics(RenderDevice * 	rd, Array< shared_ptr< Surface > > & surface, Array< shared_ptr< Surface2D > > & surface2D ) override;
    
	// Main geometry update function
    void updateChunks();

	/** Called for loading new worlds */
    void redrawWorld();

	// Applying transforms to voxels
    void selectSphere(Point3 origin, Vector3 direction);
	void selectCylinder(Point3int32 center, int radius);
    void selectBox(Point3int32 center, int radius);
    void elevateSelection(int delta);

	// Animations
    void initializeTransforms();
    void startCurrentTransform();
	void endCurrentTransform();
	void makeCrater(Point3int32 center, int radius);
    void makeShockWave(Point3 origin, Vector3 direction);
	void makeMountain(Point3int32 center, int height);
    void pullVoxelOrbit(Point3int32 origin);
};