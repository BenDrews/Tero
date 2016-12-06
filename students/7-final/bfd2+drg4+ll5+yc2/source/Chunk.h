/**
  \file Chunk.h
 */
#pragma once
#include <G3D/G3DAll.h>

/** \brief Application framework. */
class Chunk {
protected:

	Point2int32 m_chunkPos;
	Table<Point3int32, int> m_voxels;

public:

    const static int chunkSize = 32;

	Chunk(Point2int32 chunkPos);

    static Point2int32 getChunkCoords(Point3int32 pos);

	void set(Point3int32 pos, int type);

	void remove(Point3int32 pos);

	int get(Point3int32 pos);

	bool contains(Point3int32 pos);

	Table<Point3int32, int>::Iterator begin();

};