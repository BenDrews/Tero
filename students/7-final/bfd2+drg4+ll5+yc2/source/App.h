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

typedef 
    //VRApp 
    GApp
    AppBase;

class CrosshairObject{
public:
    Vector3 lookDirection;
    Point3 position;
    Ray ray;
    bool buttonSelected = false;
    int buttonIndex = 0;
    int menuControllerIndex = 0;
};


typedef 
    //VRApp 
    GApp
    AppBase; 


/** \brief Application framework. */
class App : public AppBase {
protected:

	enum VoxelType { grass, rock, brick, sand, cedar, metal, chrome, rubber };

	enum SoundIndex { add, remove, elevate, menuOpen, menuClose, menuSelect };

    typedef AppBase super;

    /** Called from onInit */
    void makeGUI();

    void initializeScene();

    const float voxelRes = 0.5f;

    const int chunkSize = 32;

    const int voxTypeCount = 8;

	Array<String> m_typesList;

	Array<AnimationControl> m_animControls;

	Array<shared_ptr<Sound>> m_sounds;

	bool lastAnimFinished = false;

  
public:

    bool vrEnabled = false;

	bool menuMode = false;
    CFrame menuFrame;
    shared_ptr<VisibleEntity> m_menu;
    const shared_ptr<ArticulatedModel>& m_menuModel = ArticulatedModel::createEmpty("menuModel");
    Array<Point3> m_menuButtons;

    int m_intersectMode = 0;
	int m_selectionMode = 1;
    bool forceIntersect = false;



    /** Camera manipulator*/
    shared_ptr<FirstPersonManipulator> m_cameraManipulator;

	/** Active voxel type */
    int m_voxelType = 0;

    /** Maps 3D chunk positions to the Tables representing each individual chunk */
    Table<Point2int32, shared_ptr<Table<Point3int32, int>>> m_posToChunk;

    /** Contains the chunk positions of the chunks that need to be updated */
    Array<Point2int32> m_chunksToUpdate;
        
	/** Maps type of voxel to Any files containing its specific properties */
	Array<Any> m_voxToProp;

	/** Maps type of voxel to Material of that voxel */
	Array<shared_ptr<UniversalMaterial>> m_voxToMat;

    /** Stores the scene model */
    const shared_ptr<ArticulatedModel>& m_model = ArticulatedModel::createEmpty("voxelModel");

    const shared_ptr<ArticulatedModel>& m_handModel = ArticulatedModel::createEmpty("handModel");

	/** Marks where the user is currently pointing */
    CrosshairObject m_crosshair;

    /** Stores the current voxel selection */
    Selection m_selection;

    /** Stores marked position when mid transform */
    Point3int32 m_currentMark;

    /** Drawing the debug voxels */
    const shared_ptr<ArticulatedModel>& m_debugModel = ArticulatedModel::createEmpty("debugModel");


    App(const super::Settings& settings = super::Settings());
    virtual void onInit() override;
    virtual bool onEvent(const GEvent& event) override;
    virtual void onUserInput(UserInput* ui) override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onGraphics(RenderDevice * 	rd, Array< shared_ptr< Surface > > & surface, Array< shared_ptr< Surface2D > > & surface2D ) override;

    void getMenuPositions();
	void initializeMaterials();
	void initializeSounds();
	void initializeModel();
	void makeMenuModel();
	const shared_ptr<ArticulatedModel> makeVoxelModel(String modelName, int type, float size = 0.5f);
    shared_ptr<VisibleEntity> addEntity(shared_ptr<ArticulatedModel> model, String entityName, bool visible=true);
    void removeEntity(shared_ptr<VisibleEntity> entity);

    void cameraIntersectVoxel(Point3int32& lastOpen, Point3int32& voxelTest);
    void updateSelect();
    void drawCrosshair(); 

	// Voxel geometry
	void addVoxel(Point3int32 input, int type);
	void removeVoxel(Point3int32 input);
    void addFace(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, Vector3 normal, Vector3::Axis axis, int type);

	// Handling chunk data structures
    Point2int32 getChunkCoords(Point3int32 pos);
    int posToVox(Point3int32 pos);
    bool voxIsSet(Point3int32 pos);
    shared_ptr<Table<Point3int32, int>> getChunk(Point3int32 pos);
    void setVoxel(Point3int32 pos, int type);
    void unsetVoxel(Point3int32 pos);
    void createVoxelGeometry(Point3int32 input);
	void drawVoxelNaive(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh, Point3 pos, float size, int type);
    void clearChunk(Point2int32 chunkPos);
    void createChunkGeometry(Point2int32 chunkPos);
    void updateChunks();
    void redrawWorld();
    void checkBoundaryAdd(Point3int32 pos);
    void updateGeometry(Point2int32 chunkCoords, int type);

	// Applying transforms to voxels
    void debugDrawVoxel();
    void selectSphere(Point3 origin, Vector3 direction);
	void selectCylinder(Point3int32 center, int radius);
    void selectBox(Point3int32 center, int radius);
    void elevateSelection(int delta);

	void makeCrater(Point3int32 center, int radius);
    void makeShockWave(Point3 origin, Vector3 direction);
	void makeMountain(Point3int32 center, int height);
    void pullVoxelOrbit(Point3int32 origin);
};