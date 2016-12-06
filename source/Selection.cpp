/** \file Selection.cpp */
#include "Selection.h"
#include "Util.h"


Array<Point3int32>& Selection::getArray(){
    return m_selection;
}

void Selection::setMark(Point3int32 mark) {
    m_mark = mark;
}

void Selection::setMode(int mode) {
	m_mode = (Mode)mode;
}

void Selection::releaseMark() {
    m_mode = idle;
}

void Selection::clear() {
    releaseMark();
    m_selection.fastClear();
}

void Selection::update(Array<Point3int32>& buffer) {
	switch (m_mode) {
	case idle :
		break;
	case regular :
		m_selection.fastClear();
		doUnion(buffer);
		break;
	case setUnion :
		doUnion(buffer);
		break;
	case setMinus :
		doMinus(buffer);
		break;
	case setIntersect :
		doIntersect(buffer);
		break;
	}
}

void Selection::doUnion(Array<Point3int32>& buffer) {
	for (int i = 0; i < buffer.size(); ++i) {
		Point3int32 current = buffer[i];

		if ( !m_selection.contains(current) ) {
			m_selection.append(current);
		}
	}
}

void Selection::doMinus(Array<Point3int32>& buffer) {

	if ( m_selection.size() == 0 ) {
		m_selection.append(buffer);
	}
	else {
		for (int i = 0; i < buffer.size(); ++i) {
			int current = m_selection.findIndex(buffer[i]);

			if ( current != -1 ) {
				m_selection.fastRemove(current);
			}
		}
	}
}

void Selection::doIntersect(Array<Point3int32>& buffer) {

	if ( m_selection.size() == 0 ) {
		m_selection.append(buffer);
	}
	else {
		for (int i = 0; i < m_selection.size(); ++i) {
			int current = buffer.findIndex(m_selection[i]);

			if ( current == -1 ) {
				m_selection.fastRemove(i);
				--i;
			}
		}
	}
}

void Selection::commitSphere(Point3 origin, Vector3 direction) {
    Point3 intersect;
    Point3 mark = Util::voxelToWorldSpace(m_mark);
    float t = Util::intersectPlane(intersect, origin, direction, mark, origin - mark);
    int radius = int((mark - intersect).magnitude() + 0.5f);
    
	Array<Point3int32> buffer;

	if (t > 0) {
        for (Point3int32 P(m_mark.x, m_mark.y - radius, m_mark.z); P.y <= m_mark.y + radius; ++P.y) {
            for (P.x = m_mark.x - radius; P.x <= m_mark.x + radius; ++P.x) {
                for (P.z = m_mark.z - radius; P.z <= m_mark.z + radius; ++P.z) {

		    		// check if the voxel is in the sphere
                    if(sqrt((P.x - m_mark.x) * (P.x - m_mark.x) + (P.y - m_mark.y) * (P.y - m_mark.y) + (P.z - m_mark.z) * (P.z - m_mark.z)) <= radius) {
                        buffer.append(P);
                    }
                }
            }
        }
    }

	update(buffer);
}

void Selection::commitCylinder(Point3int32 center, int radius) {

	Array<Point3int32> buffer;

    for (Point3int32 P(center.x, center.y - radius, center.z); P.y <= center.y + radius; ++P.y) {
        for (P.x = center.x - radius; P.x <= center.x + radius; ++P.x) {
            for (P.z = center.z - radius; P.z <= center.z + radius; ++P.z) {

				// check if the voxel is in the cylinder
                if ( sqrt((P.x-center.x) * (P.x-center.x) + (P.z-center.z) * (P.z-center.z)) <= radius ) {
                    buffer.append(P);
                }
            }
        }
    }

	update(buffer);
}


void Selection::commitBox(Point3int32 center, int radius) {

	Array<Point3int32> buffer;

    for (Point3int32 P(center.x, center.y - radius, center.z); P.y <= center.y + radius; ++P.y) {
        for (P.x = center.x - radius; P.x <= center.x + radius; ++P.x) {
            for (P.z = center.z - radius; P.z <= center.z + radius; ++P.z) {
				buffer.append(P);
            }
        }
    }

	update(buffer);
}