#pragma once
#include <G3D/G3DAll.h>
/**
  \file Util.h

  Class with useful utility functions.
 */
class Util {
	public:
        static float intersectPlane(Point3& intersect, Point3 origin, Vector3 direction, Point3 point, Vector3 normal);
};