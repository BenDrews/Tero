/** \file Selection.cpp */
#include "Selection.h"
#include "Util.h"


Array<Point3int32>& Selection::getArray(){
    return m_selection;
}

void Selection::setMark(Point3int32 mark) {
    m_mark = mark;
    m_mode = 1;
}

void Selection::releaseMark() {
    m_mode = 0;
}

void Selection::clear() {
    releaseMark();
    m_selection.fastClear();
}

void Selection::commitSphere(Point3 origin, Vector3 direction) {
    Point3 intersect;
    Point3 mark = Util::voxelToWorldSpace(m_mark);
    float t = Util::intersectPlane(intersect, origin, direction, mark, origin-mark);
    int radius = int((mark-intersect).magnitude() + 0.5f);
    if (t > 0) {
        
        for (int y = m_mark.y - radius; y <= m_mark.y + radius; ++y) {
            for (int x = m_mark.x - radius; x <= m_mark.x + radius; ++x) {
                for (int z = m_mark.z-radius; z <= m_mark.z + radius; ++z) {
                    Point3int32 pos = Point3int32(x,y,z);

		    		// check if the voxel is in the sphere
                    if(sqrt((x-m_mark.x) * (x-m_mark.x) + (y-m_mark.y) * (y-m_mark.y) + (z-m_mark.z) * (z-m_mark.z)) <= radius) {
                        m_selection.append(pos);
                    }
                }
            }
        }
    }
}