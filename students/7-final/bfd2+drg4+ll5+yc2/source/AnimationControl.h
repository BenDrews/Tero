/**
  \file AnimationControl.h
 */
#pragma once
#include <G3D/G3DAll.h>
#include <GLG3DVR/VRApp.h>

class AnimationControl {

protected:

	SimTime m_currentTime;//Should be Any;

	std::function<void(SimTime, SimTime, shared_ptr<Table<String, float>>, Table< Point3int32 ,shared_ptr<VisibleEntity>>&, Table< Point3int32, SimTime>& )> m_callback;

public:
	
	shared_ptr<Table<String, float>> args;

    Table<Point3int32, shared_ptr<VisibleEntity>> activeVoxels;
    
	Table<Point3int32, SimTime> timeAdded;

	AnimationControl();

	// c is the function that will be called by onSimulation
	AnimationControl(std::function<void(SimTime, SimTime, shared_ptr<Table<String, float>>, Table< Point3int32, shared_ptr<VisibleEntity>>&, Table< Point3int32, SimTime>& )> c);

	void invoke(SimTime sdt);
    
};