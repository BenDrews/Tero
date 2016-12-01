/** \file Util.cpp */
#include "Util.h"


static float intersectPlane(Point3& intersect, Point3 origin, Vector3 direction, Point3 point, Vector3 normal) {
    float denom = dot(direction, normal);
    if(abs(denom) < 1e-6) {
        return false;
    } else {
        float dist = (dot(point-origin, normal) / denom);
        intersect = origin + (dist * direction);
        return dist;
    }
}