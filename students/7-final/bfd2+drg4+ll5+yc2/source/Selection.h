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

	/** Voxel positions in current selection*/
	Array<Point3int32> m_selection;
	/** Used in `commitSphere` to store the center of the sphere. */
    Point3int32 m_mark;
    /** Current intersection mode */
	Mode m_mode = idle;
	
public:

	Array<Point3int32>& getArray();
	void setMark(Point3int32 mark);
	void setMode(int mode);
	void releaseMark();
	void clear();

	/** Calls `doUnion`, `doMinus`, or `doIntersect` depending on `m_mode` */
	void update(Array<Point3int32>& buffer);
	/** Performs set union with `m_selection` and buffer */
	void doUnion(Array<Point3int32>& buffer);
	/** Performs set minus with `m_selection` and buffer */
	void doMinus(Array<Point3int32>& buffer);
	/** Performs set intersection with `m_selection` and buffer */
	void doIntersect(Array<Point3int32>& buffer);

	/** Updates `m_selection` with a buffer of voxels contained in a sphere with center at `m_selection`'s mark and radius determined by given ray. */
	void commitSphere(Point3 origin, Vector3 direction);
	/** Updates `m_selection` with a buffer of voxels contained in a cylinder determined by the given center and radius */
	void commitCylinder(Point3int32 center, int radius);
	/** Updates `m_selection` with a buffer of voxels contained in a box determined by the given center and radius (half an edge length) */
	void commitBox(Point3int32 center, int radius);
};