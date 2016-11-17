/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include <GLG3DVR/VRApp.h>




class PlayerCamera {
    public:
    CFrame& position;
    float speed;
    float headTilt; //Radians
    float heading;
    float desiredYaw;
    float desiredPitch;
    Vector3 desiredOS;

    PlayerCamera();
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

    const int voxTypeCount = 2;

  
public:
	/** Maps 3D positions to ints that denote the type of voxel */
	Table<Point3int32, int> m_posToVox;
        
	/** Maps type of voxel to Any files containing its specific properties */
	Table<int, Any> m_voxToProp;

	/** Maps type of voxel to Material of that voxel */
	Table<int, shared_ptr<UniversalMaterial>> m_voxToMat;

    /** Stores the scene model */
    const shared_ptr<ArticulatedModel>& m_model = ArticulatedModel::createEmpty("voxelModel");;

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

    void cameraIntersectVoxel(Point3int32& lastOpen, Point3int32& voxelTest);


    PlayerCamera player;
    void initializePlayer();
    void movePlayer(SimTime deltaTime);
    bool m_firstPersonMode = true;
    BoxShape crossHair;

	shared_ptr<Model> initializeModel();
	void initializeMaterials();
    void addVoxelModelToScene();

	int normalToFace(Vector3 n);
	void addVoxel(Point3int32 input, int type);
	void removeFace(ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh);
	void removeVoxel(Point3int32 input);
    void addFace(Point3int32 pos, Vector3 normal, Vector3::Axis axis, int type);
    float maxDistGrid(Point3 pos, Vector3 dir);

    void selectCircle(Point3int32 center, int radius);
    void App::elevateSelection(int delta);
};


