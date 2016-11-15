/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include "Mesh.h"

/** \brief Application framework. */
class App : public GApp {
protected:

    /** Called from onInit */
    void makeGUI();
  
public:
	/** Maps 3D positions to ints that denote the type of voxel */
	Table<Point2int32, int> m_posToVox;
	/** Maps type of voxel to Any files containing its specific properties */
	Table<int, Any> m_voxToProp;

    App(const GApp::Settings& settings = GApp::Settings());
    virtual void onInit() override;
	shared_ptr<Model> initializeModel();
    void App::addVoxelModelToScene();
};