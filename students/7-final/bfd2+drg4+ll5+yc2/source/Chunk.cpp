/** \file Chunk.cpp */
#include "Chunk.h"

Chunk::Chunk(Point2int32 chunkPos) {
	m_chunkPos = chunkPos;
}

// Returns the chunk coords for a given point.
Point2int32 Chunk::getChunkCoords(Point3int32 pos) {
	// Chunks between -7 and 7 are too big, modify
    int32 addX = 0;
    int32 addZ = 0;
    if (pos.x < 0) {
        addX = -1;
    }
    if (pos.z < 0) {
        addZ = -1;
    }
    return Point2int32((pos.x / chunkSize) + addX, (pos.z / chunkSize) + addZ);
}

void Chunk::set(Point3int32 pos, int type) {
	m_voxels.set(pos, type);
}

void Chunk::remove(Point3int32 pos) {
	m_voxels.remove(pos);
}

int Chunk::get(Point3int32 pos) {
    return m_voxels[pos];
}

bool Chunk::contains(Point3int32 pos) {
	return m_voxels.containsKey(pos);
}

Table<Point3int32, int>::Iterator Chunk::begin() {
	return m_voxels.begin();
}