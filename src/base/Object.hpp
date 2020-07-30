#pragma once

#include <embree3/rtcore.h>
#include <tinyply.h>
#include <glm/glm.hpp>

#include <fstream>
#include <string>
#include <limits>

class Object {
public:
    typedef glm::f32vec3 Vertex;
    typedef glm::u32vec3 Face;

    struct Box {
        Vertex boxMin;
        Vertex boxMax;
        Vertex center;
        Vertex extent;
        float maxExtent;
        float minExtent;
    };

    Object(RTCDevice device, RTCScene scene, const std::string &path);

    Object(
            RTCDevice device,
            RTCScene scene,
            Vertex *vertices,
            size_t vertexCount,
            Face *faces,
            size_t faceCount
    );

    ~Object();

    RTCGeometry getGeometry() const;
    unsigned int getGeometryID() const;
    Vertex *getVertices() const;
    Face *getFaces() const;
    size_t getVertexCount() const;
    size_t getFaceCount() const;
    const Box &getAABB() const;

    //void setVertex(size_t index, const Vertex &vertex);
    //void setFace(size_t index, const Face &face);

private:
    void create(
            RTCDevice device,
            RTCScene scene,
            Vertex *vertices,
            size_t vertexCount,
            Face *faces,
            size_t faceCount
    );

    RTCGeometry m_geometry = nullptr;
    unsigned int m_geometryID = 0;
    Vertex *m_vertices = nullptr;
    Face *m_faces = nullptr;
    size_t m_vertexCount = 0;
    size_t m_faceCount = 0;

    Box m_aabb = {
            {
                    std::numeric_limits<float>::infinity(),
                    std::numeric_limits<float>::infinity(),
                    std::numeric_limits<float>::infinity()
            },
            {
                    -std::numeric_limits<float>::infinity(),
                    -std::numeric_limits<float>::infinity(),
                    -std::numeric_limits<float>::infinity()
            }
    };
};
