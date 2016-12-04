#pragma once
#include <G3D/G3DAll.h>
/**
  \file Util.h

  Class with useful utility functions.
 */
class Util {

public:
    static Point3 Util::voxelToWorldSpace(Point3int32 voxelPos);
    static Point3int32 Util::worldToVoxelSpace(Point3 worldPos);
    static float intersectPlane(Point3& intersect, Point3 origin, Vector3 direction, Point3 point, Vector3 normal);
};
    
static const float voxelRes = 0.5f;