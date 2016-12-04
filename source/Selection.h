#pragma once
#include <G3D/G3DAll.h>
/**
  \file Selection.h

  Class that handles manipulating the selection.
 */

class Selection {
	public:
        Array<Point3int32> m_selection;
        Point3int32 m_mark = Point3int32();
        int m_mode = 0;
        
        Array<Point3int32>& getArray();
        void setMark(Point3int32 mark);
        void releaseMark();
        void clear();
        void commitSphere(Point3 origin, Vector3 direction);
};