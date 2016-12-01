#pragma once
#include <G3D/G3DAll.h>
/**
  \file Selection.h

  Class that handles manipulating the selection.
 */
#include "App.h"

class Selection {
	public:
        Array<Point3> m_selection;
        Point3 m_mark = Point3();
        int m_mode = 0;
        
        void setMark(Point3 mark);
        void releaseMark();
        void commitSphere(Point3 origin, Vector3 direction);
};