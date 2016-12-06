/**
  \file Selection.h

  Class that handles manipulating the selection.
 */
#pragma once
#include <G3D/G3DAll.h>
#include "Util.h"

class Selection {

	enum Mode { idle, regular, setUnion, setMinus, setIntersect };
	
protected:

	Array<Point3int32> m_selection;
    Point3int32 m_mark = Point3int32();
    Mode m_mode = idle;
	
public:
        
	Array<Point3int32>& getArray();
	void setMark(Point3int32 mark);
	void setMode(int mode);
	void releaseMark();
	void clear();

	void update(Array<Point3int32>& buffer);
	void doUnion(Array<Point3int32>& buffer);
	void doMinus(Array<Point3int32>& buffer);
	void doIntersect(Array<Point3int32>& buffer);

	void commitSphere(Point3 origin, Vector3 direction);
	void commitCylinder(Point3int32 center, int radius);
	void commitBox(Point3int32 center, int radius);
};