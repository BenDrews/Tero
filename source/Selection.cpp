/** \file Selection.cpp */
#include "Selection.h"
#include "Util.h"


void Selection::setMark(Point3 mark) {
    m_mark = mark;
    m_mode = 1;
}

void Selection::releaseMark() {
    m_mode = 0;
}

void Selection::commitSphere(Point3 origin, Vector3 direction) {
    Point3 intersect;
    Util::intersectPlane(intersect, origin, direction, m_mark, m_mark-origin) {
        
    }
}