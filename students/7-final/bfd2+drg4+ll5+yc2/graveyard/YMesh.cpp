#pragma once
#include "YApp.h"
#include "YMesh.h"

/**starter function that takes in a ParseVOX and calls other methods to save it as an OBJ file.*/
void Mesh::initFromVoxels(const ParseVOX& voxelGrid, String m_outputName){
    
    Table<int, int> texcoordtable;
    const Array<ParseVOX::Voxel>& voxelarray = voxelGrid.voxel;
    //int l = voxelarray.length();
    Set<Point3> occupied;
    
    for (const ParseVOX::Voxel& voxel : voxelarray) {
        Point3 position(voxel.position.x,voxel.position.y,voxel.position.z);
        occupied.insert(position);
    }
    for (const ParseVOX::Voxel& voxel : voxelarray) {
        Point2 texCoord((float(voxel.index) - 0.5f) / 256.0f,0.5f);
        Point3 position(voxel.position.x,voxel.position.y,voxel.position.z);
        Vector3 vecposition(position.x, position.y, position.z);
        if(!occupied.contains(position+ Vector3(1, 0, 0))){
            Vector3::Axis axis = Vector3::X_AXIS;
            float sign = 1.0f;
            addQuad(axis, position, sign, texCoord);
        }
        if(!occupied.contains(position+ Vector3(-1, 0, 0))){
            Vector3::Axis axis = Vector3::X_AXIS;
            float sign = -1.0f;
            addQuad(axis, position, sign, texCoord);
        }
        if(!occupied.contains(position+ Vector3(0, 1, 0))){
            Vector3::Axis axis = Vector3::Y_AXIS;
            float sign = 1.0f;
            addQuad(axis, position, sign, texCoord);
        }
        if(!occupied.contains(position+ Vector3(0, -1, 0))){
            Vector3::Axis axis = Vector3::Y_AXIS;
            float sign = -1.0f;
            addQuad(axis, position, sign, texCoord);
        }
        if(!occupied.contains(position+ Vector3(0, 0, 1))){
            Vector3::Axis axis = Vector3::Z_AXIS;
            float sign = 1.0f;
            addQuad(axis, position, sign, texCoord);
        }
        if(!occupied.contains(position+ Vector3(0, 0, -1))){
            Vector3::Axis axis = Vector3::Z_AXIS;
            float sign = -1.0f;
            addQuad(axis, position, sign, texCoord);
        }
    }
    weldVertices();
    TextOutput myfile("../data-files/model/"+m_outputName+".obj");
    saveToOBJ(myfile,m_outputName);
}

/**helper function that computes the 4 vertices of a face and the texture coordinates using the center coordinate of the voxel.
Called from Mesh::initFromVoxels()*/

void Mesh::addQuad(Vector3::Axis axis, const Point3& center, float sign, const Point2& texCoord) {
    Vector3 normal = Vector3::zero();
    normal[axis] = sign;

    Vector3 uAxis = Vector3::zero();
    Vector3 vAxis = Vector3::zero();
    uAxis[(axis + 1) % 3] = 1;
    vAxis[(axis + 2) % 3] = 1;

    Point3 A = center + normal * 0.5f + uAxis * 0.5f - vAxis * 0.5f;
    Point3 B = center + normal * 0.5f + uAxis * 0.5f + vAxis * 0.5f;
    Point3 C = center + normal * 0.5f - uAxis * 0.5f + vAxis * 0.5f;
    Point3 D = center + normal * 0.5f - uAxis * 0.5f - vAxis * 0.5f;
    if(sign>0.0f){
    addQuad(A,B,C,D,texCoord);
    }else{
    addQuad(A,D,C,B,texCoord);
    }
}

/**helper function that takes in 4 vertices of a face and updating indexArray,textureCoordinates and vertexArray.
Called from Mesh::addQuad()*/
void Mesh::addQuad(const Point3& A, const Point3& B, const Point3& C, const Point3& D,  const Point2& texCoord){
    const int size = vertexArray.size();
    vertexArray.append(A,B,C,D);
   
    for (int a = 0; a < 3; ++a) {
                indexArray.append(size + a);
                textureCoords.append(texCoord);
            }
            indexArray.append(size);
            textureCoords.append(texCoord);
            for (int a = 2; a < 4; ++a) {
                indexArray.append(size + a);
            }
}

/**helper function that welds the vertices and updates vertexArray, textureCoords, normalArray, indexArray.
Called from Mesh::initFromVoxels()*/
void Mesh::weldVertices(){
    Welder::Settings settings;
    Welder::weld(vertexArray, textureCoords, normalArray, indexArray, settings);
}

/**helper function that saves to an OBJ file using vertexArray, textureCoords, normalArray, indexArray.
Called from Mesh::initFromVoxels()*/
void Mesh::saveToOBJ(TextOutput myfile, String m_outputName){
    myfile.printf("\n# material\nmtllib "+m_outputName+".mtl\nusemtl palette \n");
    Table<Vector3, int> vertexMap;
    Table<Vector3, int> normalMap;
    Table<Vector2, int> textureMap;
    for (const Vector3& position : vertexArray) {
        if (!vertexMap.containsKey(position)) {
            myfile.printf("v %f %f %f \n", position.y, position.z, position.x);
            vertexMap.set(position, int(vertexMap.size()));
        }
    }
    for (const Vector2& texture : textureCoords) {

        if (!textureMap.containsKey(texture)) {
            myfile.printf("vt %f %f \n", texture.x, texture.y);
            textureMap.set(texture, int(textureMap.size()));
           
        }
    }
    for (const Vector3& normal : normalArray) {

        if (!normalMap.containsKey(normal)) {
            myfile.printf("vn %f %f %f \n", normal.y,normal.z, normal.x);
            normalMap.set(normal, int(normalMap.size()));
        }
    }

    //Printing
    int size = indexArray.size();

    for (int i = 0; i < size; i += 3) {

        myfile.printf("f %d/%d/%d %d/%d/%d %d/%d/%d \n",
            vertexMap.get(vertexArray[indexArray[i] ])+1,
            textureMap.get(textureCoords[indexArray[i] ])+1,
            normalMap.get(normalArray[indexArray[i]])+1,

            vertexMap.get(vertexArray[indexArray[i + 1] ])+1,
            textureMap.get(textureCoords[indexArray[i + 1]])+1,
            normalMap.get(normalArray[indexArray[i + 1] ])+1,

            vertexMap.get(vertexArray[indexArray[i + 2]])+1,
            textureMap.get(textureCoords[indexArray[i + 2]])+1,
            normalMap.get(normalArray[indexArray[i + 2]])+1

            

            

            

            
        );
    }
    myfile.commit();


}