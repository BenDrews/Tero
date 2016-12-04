/**
  \file AnimationControl.h
 */
#pragma once
#include <G3D/G3DAll.h>
#include <GLG3DVR/VRApp.h>

class AnimationControl {

protected:

	SimTime m_currentTime;//Should be Any;
	std::function<void(SimTime, SimTime, Table<String, float>)> m_callback;

public:
	
	Table<String, float> args;
    Array<shared_ptr<VisibleEntity>> activeVoxels;
	AnimationControl();

	// c is the function that will be called by onSimulation
	AnimationControl(std::function<void(SimTime, SimTime, Table<String, float>)> c);

	void invoke(SimTime sdt);
    
};