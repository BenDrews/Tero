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
public:

protected:

    /** Called from onInit */
    void makeGUI();
  
public:
    App(const GApp::Settings& settings = GApp::Settings());
    virtual void onInit() override;
};