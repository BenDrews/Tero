/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include <GLG3DVR/VRApp.h>


class SelectionObject{
public:
    Vector3 lookDirection;
    Point3 position;
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

    const int voxTypeCount = 2;

  
public:

    int m_voxelType = 0;
    /** VR Toggle**/
    bool vrEnabled = false;

    /** Camera manipulator*/
    shared_ptr<FirstPersonManipulator> m_cameraManipulator;

    /** Maps 3D chunk positions to the Tables representing each individual chunk */
    Table<Point2int32, shared_ptr<Table<Point3int32, int>>> m_posToChunk; 

    /** Contains the chunk positions of the chunks that need to be updated */
    Array<Point2int32> m_chunksToUpdate;
        
	/** Maps type of voxel to Any files containing its specific properties */
	Table<int, Any> m_voxToProp;

	/** Maps type of voxel to Material of that voxel */
	Table<int, shared_ptr<UniversalMaterial>> m_voxToMat;

    /** Stores the scene model */
    const shared_ptr<ArticulatedModel>& m_model = ArticulatedModel::createEmpty("voxelModel");

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
    void cameraIntersectVoxel(Point3int32& lastOpen, Point3int32& voxelTest);
    void makeFP();
    void updateSelect();
    void drawSelection();
    SelectionObject select;

	void initializeModel();
	void initializeMaterials();
    void addVoxelModelToScene();

    Point2int32 getChunkCoords(Point3int32 pos);
    shared_ptr<Table<Point3int32, int>> getChunk(Point3int32 pos);
    void setVoxel(Point3int32 pos, int type);
    void unsetVoxel(Point3int32 pos);
    void drawVoxel(Point3int32 input);
    void redrawChunk(Point2int32 chunkPos);
    void updateChunks();
    void redrawWorld();

	void addVoxel(Point3int32 input, int type);
	void removeVoxel(Point3int32 input);
    void addFace(Point3int32 pos, Vector3 normal, Vector3::Axis axis, int type);
    float maxDistGrid(Point3 pos, Vector3 dir);
    int posToVox(Point3int32 pos);

    void selectCircle(Point3int32 center, int radius);
    void App::elevateSelection(int delta);

    Vector3 m_offset = Vector3(0.05,0,4);

};