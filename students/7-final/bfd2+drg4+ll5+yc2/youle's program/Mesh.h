#pragma once
#include <G3D/G3DAll.h>
class Mesh {
public:
    Array<Vector3> vertexArray;
    Array<Vector2> textureCoords;
    Array<Vector3> normalArray;
    Array<int>     indexArray;


    void initFromVoxels(const ParseVOX& voxelGrid,String m_outputName);
    void addQuad(Vector3::Axis axis, const Point3& center, float sign, const Point2& texCoord);
    void addQuad(const Point3& A, const Point3& B, const Point3& C, const Point3& D,  const Point2& texCoord);
    void weldVertices();
    void saveToOBJ(TextOutput myfile,String m_outputName);
};
