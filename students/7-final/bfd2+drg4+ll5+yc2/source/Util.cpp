/** \file Util.cpp */
#include "Util.h"


float Util::intersectPlane(Point3& intersect, Point3 origin, Vector3 direction, Point3 point, Vector3 normal) {
    float denom = dot(direction, normal);
    if(abs(denom) < 1e-6) {
        return -1.0f;
    } else {
        float dist = (dot(point-origin, normal) / denom);
        intersect = origin + (dist * direction);
        return dist; 
    }
}

// Conversions between voxelspace and worldspace, the hard coded point3 is because voxels are drawn in center 
Point3 Util::voxelToWorldSpace(Point3int32 voxelPos) {
    return Point3(voxelPos) * voxelRes + Point3(0.5, 0.5f, 0.5f);
}

//Ditto
Point3int32 Util::worldToVoxelSpace(Point3 worldPos){
    return (Point3int32)((worldPos - Point3(0.5, 0.5f, 0.5f)) / voxelRes);
}