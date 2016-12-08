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

	/** Edge length of voxels in the world */
    const float voxelRes = 0.5f;
	/** Number of voxel types displayed per menu page */
	const int menuPageSize = 12;
	int numMenuPages;
    /** Number of naive colors in rainbow voxels */
    const int rainbowSize = 100;
	/** Number of voxel types in the imported model */
    const int palSize = 256;

	/** Number of textured voxel types. Excludes number of procedurally generated colored voxel types. */
    const int numTexturedVoxels = 40;

	/** Total number of voxel types. Initialized as number of textured voxels and incremented in `addColorMaterials()` and `importMagicaVox()`. */
    int voxTypeCount = numTexturedVoxels;

	/** 0 is VoxelMenu, 1 is TransformMenu */
    int menuType = 0;

	/** List of transforms */
    Array<String> m_transforms;

    String currentTransform = "shockwave";

	bool lastAnimFinished = false;

	// Options
    bool vrEnabled = false;
	/** True if menu is visible */
	bool menuMode = false;
	/** If false, crosshair will try to intersect voxels in the scene */
    bool forceIntersect = false;
	/** Distance of intersection */
    int m_intersectMode = 1;
	/** Active selection mode (0 = idle, 1 = regular, 2 = set union, 3 = set minus, 4 = set intersection */
	int m_selectionMode = 1;
	int m_currentMenuPage = 0;
	/** Active voxel type */
    int m_voxelType = 0;

    /** Is there currently a voxel orbit */
    bool m_hasOrbit = false;

	/** Initial position for the voxel throw in voxel orbit */
	Point3  m_throwStartPos;

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

	/** Stores animations that are currently being executed */
	Array<AnimationControl> m_animControls;

	Array<shared_ptr<Sound>> m_sounds;

	// Objects in the world
    /** Stores the scene model */
    const shared_ptr<ArticulatedModel>& m_model = ArticulatedModel::createEmpty("voxelModel");

	/** Model for hand in Virtual Reality */
    const shared_ptr<ArticulatedModel>& m_handModel = ArticulatedModel::createEmpty("handModel");
    shared_ptr<VisibleEntity> m_hand1;
    shared_ptr<VisibleEntity> m_hand2;

	/* Returns Player frame regardless of whether or not vr is enabled*/
	CFrame getPlayerFrame();

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

    /** Called from onInit */
    void makeGUI();
    /** Called from onInit */
    void initializeScene();
    /** Called from onInit */
	void initializeMaterials();
    /** Called from onInit */
	void initializeSounds();
    /** Called from onInit */
	void initializeModel();
    /** Called from onInit */
    void initializeHands();
	/** Called from onInit */
    void initializeTransforms();

	// Menu functions
    /** Called from onInit */
    void getMenuPositions();
    /** Called from onInit */
	void makeMenuPageModels();
    /** Called from onInit */
	void makeMenuPageEntities();
    void setMenuFrame();
	void changeMenuPage(int to);

    void updateHands(int index);

	/** Generates voxel types that are solid colors. */
    void addColorMaterials();

	const shared_ptr<ArticulatedModel> makeVoxelModel(String modelName, int type, float size = 0.5f);

	/** Adds a VisibleEntity with the given ArticulatedModel and entity name to the scene. Entity defaults to visible. */
    shared_ptr<VisibleEntity> addEntity(shared_ptr<ArticulatedModel> model, String entityName, bool visible = true);
    void removeEntity(shared_ptr<VisibleEntity> entity);

	// Manipulating the Chunk data structure
    int posToVox(Point3int32 pos);

    /** Main functions for manipulating the data of the world */
	void setVoxel(Point3int32 pos, int type);
    void unsetVoxel(Point3int32 pos);
    bool voxIsSet(Point3int32 pos);
    shared_ptr<Chunk> getChunk(Point3int32 pos);

	/** Create geometry for a single voxel. Does not add a face if it finds an adjacent face. */
    void createVoxelGeometry(Point3int32 input);

    /** Create every face for a voxel */
	void createNaiveVoxelGeometry(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, int type);

    /** Create the geometry for a single face */
	void addFace(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, Vector3 normal, Vector3::Axis axis, int type);
	
    /** Calls setVoxel and createVoxelGeometry */
    void addVoxel(Point3int32 input, int type);

    /** Calls unsetVoxel and marks the chunk as needing an update */
	void removeVoxel(Point3int32 input);

    /** Marks a chunk for updating */
    void markChunkDirty(Point3int32 pos);
	/** Clears CPU vertex and CPU index arrays for every chunk type. Called from updateChunks() */
    void clearChunkGeometry(Point2int32 chunkPos);
	/** Create geometry for a given chunk. Called from updateChunks() */
    void createChunkGeometry(Point2int32 chunkPos);
	/** Called from createChunkGeometry() and addVoxel() */
    void cleanChunkGeometry(Point2int32 chunkCoords, int type);

	/** Used explicitly for removing voxels, check whether the voxel is the boundary of a chunk.
		If it is, push the neighboring chunk to the m_chunksToRedraw for later update. 
		Can add up to 2 more chunks for update. */
	void checkBoundaryAdd(Point3int32 pos);

	/** Visualize the user's currently selected voxels. */
    void drawSelectionPreview();

	/** Intersects the scene by ray marching with the ray stored in `m_crosshair`. Returns the voxel and previous voxel intersected. */
    void cameraIntersectVoxel(Point3int32& lastOpen, Point3int32& voxelTest);
	/** Determine ray to store in `m_crosshair` based on the position of the first controller in the VR controller array. */
    Ray getVrCrosshairRay();
	/** Determine ray to store in `m_crosshair` based on mouse position. */
    Ray getMouseCrosshairRay();
    void updateCrosshair();
	/** Determines whether or not a menu button has been selected. If so, it updates `m_crosshair`, setting the `buttonSelected` variable equal to true and storing the button that has been selected. */
    void getMenuCrosshairRay();
    void updateCrosshairRay(Ray crosshairRay);
	/** Visualizes the crosshair by drawing a debug box where the crosshair intersects the scene. The face adjacent to the previous voxel intersected is colored blue. */
    void drawCrosshair();

public:

    App(const super::Settings& settings = super::Settings());
    virtual void onInit() override;
    virtual bool onEvent(const GEvent& event) override;
    virtual void onUserInput(UserInput* ui) override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onGraphics(RenderDevice * 	rd, Array< shared_ptr< Surface > > & surface, Array< shared_ptr< Surface2D > > & surface2D ) override;
    
	/** Main geometry update function */
    void updateChunks();

	/** Called for loading new worlds */
    void redrawWorld();

	// Applying transforms to voxels
	/** Updates `m_selection` with sphere with center at `m_selection`'s mark and radius determined by given ray */
    void selectSphere(Point3 origin, Vector3 direction);
	/** Updates `m_selection` with cylinder with given center and radius, and height equal to diameter */
	void selectCylinder(Point3int32 center, int radius);
	/** Updates `m_selection` with box with given center and radius (half an edge length) */
    void selectBox(Point3int32 center, int radius);
	/** Translates selected voxels by given height. */
    void elevateSelection(int delta);

	// Animations
	/** Gets the first input to the transform that is selected from the Virtual Reality menu. */
    void setUpCurrentTransform();
	/** Gets the second input to the transform that is selected from the Virtual Reality menu and starts executing it. */
	void commitCurrentTransform();
	/** Creates an `AnimationControl` for making a crater with center and radius determined by user input.
	    Voxels are removed gradually starting at the center and ending at the outer ring. */
	void makeCrater(Point3int32 center, int radius);
	/** Creates an `AnimationControl` for making a shock wave that causes voxels in the scene to move up and down in a wave-like motion. */
    void makeShockWave(Point3 origin, Vector3 direction);
	/** Creates an `AnimationControl` for making a mountain with center and height determined by user input.
		Voxels are added gradually from the bottom to the top. */
	void makeShockWaveArc(Point3int32 origin, Vector3 direction);
	/** Creates an `AnimationControl` for making a mountain with center and height determined by user input.
		Voxels are added gradually from the bottom to the top. */
	void makeMountain(Point3int32 center, int height);
	/** Creates an `AnimationControl` that tears voxels away from the world and makes them orbit the user. */
    void pullVoxelOrbit(Point3int32 origin);
	/** Edits the arguments in a `pullVoxelOrbit AnimationControl` and allows the user to fling orbiting voxels in different directions. */
    void flingSatellite(Vector3 dir, float speed);

	/** Import MagicaVoxel files */
    void importVoxFile();
};