/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include <GLG3DVR/VRApp.h>


class crosshairObject{
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

    typedef AppBase super;

    /** Called from onInit */
    void makeGUI();

    void initializeScene();

    const float voxelRes = 0.5f;

    const int chunkSize = 8;

    const int voxTypeCount = 8;

  
public:

    int m_voxelType = 0;
    /** VR Toggle**/
    bool vrEnabled = false;



    bool menuMode = false;
    CFrame menuFrame;
    shared_ptr<Entity> m_menu;
    Array<Point3> m_menuButtons;

   
    int intersectMode=0;
    bool forceIntersect = false;

    /** Camera manipulator*/
    shared_ptr<FirstPersonManipulator> m_cameraManipulator;

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

	/** ArticulatedModel for menu */
    const shared_ptr<ArticulatedModel>& m_menuModel = ArticulatedModel::createEmpty("menuModel");
	void addMenuFace(Point3 center, Vector3 normal, Vector3::Axis axis, int type);

    /** Store the current voxel selection */
    Array<Point3int32> m_selection;

    /** Store marked position when mid transform */
    Point3int32 m_currentMark;

    App(const super::Settings& settings = super::Settings());
    virtual void onInit() override;
    virtual bool onEvent(const GEvent& event) override;
    virtual void onUserInput(UserInput* ui) override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onGraphics(RenderDevice * 	rd, Array< shared_ptr< Surface > > & surface, Array< shared_ptr< Surface2D > > & surface2D ) override;

    Point3 voxelToWorldSpace(Point3int32 voxelPos);
    Point3int32 worldToVoxelSpace(Point3 worldPos);

    void cameraIntersectVoxel(Point3int32& lastOpen, Point3int32& voxelTest);
    void updateSelect();
    void drawSelection();
    crosshairObject crosshair;

	void initializeModel();
	void makeMenuModel();
    void initializeMenu();
    void getMenuPositions();
	void initializeMaterials();
    void addModelToScene(shared_ptr<ArticulatedModel> model, String entityName);
    


    Point2int32 getChunkCoords(Point3int32 pos);
    bool voxIsSet(Point3int32 pos);
    shared_ptr<Table<Point3int32, int>> getChunk(Point3int32 pos);
    void setVoxel(Point3int32 pos, int type);
    void unsetVoxel(Point3int32 pos);
    void drawVoxel(Point3int32 input);
    void clearChunk(Point2int32 chunkPos);
    void drawChunk(Point2int32 chunkPos);
    void updateChunks();
    void redrawWorld();
    void checkBoundaryAdd(Point3int32 pos);


	void addVoxel(Point3int32 input, int type);
	void removeVoxel(Point3int32 input);
    void addFace(Point3int32 pos, Vector3 normal, Vector3::Axis axis, int type);
    float maxDistGrid(Point3 pos, Vector3 dir);
    int posToVox(Point3int32 pos);

    void selectCircle(Point3int32 center, int radius);
    void App::elevateSelection(int delta);

    Vector3 m_controllerOffset = Vector3(0,0,0);//Vector3(0.05,0,4);
    Vector3 m_sceneOffset = Vector3(0,0,0);
    void updateGeometry( Point2int32 chunkCoords,int type);

};