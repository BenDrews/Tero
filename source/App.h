/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include <GLG3DVR/VRApp.h>

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

    const int voxTypeCount = 1;
  
public:
	/** Maps 3D positions to ints that denote the type of voxel */
	Table<Point3int32, int> m_posToVox;
        
	/** Maps type of voxel to Any files containing its specific properties */
	Table<int, Any> m_voxToProp;

	/** Maps type of voxel to Material of that voxel */
	Table<int, shared_ptr<UniversalMaterial>> m_voxToMat;

    /** Stores the scene model */
    const shared_ptr<ArticulatedModel>& m_model = ArticulatedModel::createEmpty("voxelModel");;

    App(const super::Settings& settings = super::Settings());
    virtual void onInit() override;
	shared_ptr<Model> initializeModel();
	void initializeMaterials();
    void addVoxelModelToScene();

	void addVoxel(Point3int32 input, int type);
	void removeVoxel(Point3int32 input);
    void addFace(Point3int32 input, Vector3 normal, int type, ArticulatedModel::Geometry* geometry, ArticulatedModel::Mesh* mesh);

};