#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <string>

struct FaceData
{
    int vertexIndex[3];
    int texCoordIndex[3];
    int normalIndex[3];
};

class GeometryData
{
public:
    void loadFromOBJFile(std::string filename);

    int vertexCount() const;

    void* vertexData() const;
    void* textureCoordData() const;
    void* normalData() const;
    void* tangentData() const;
    void* bitangentData() const;

private:
    std::vector<float> vertices;
    std::vector<float> textureCoords;
    std::vector<float> normals;
    std::vector<float> tangents;
    std::vector<float> bitangents;

    std::vector<FaceData> faces;
};

#endif
