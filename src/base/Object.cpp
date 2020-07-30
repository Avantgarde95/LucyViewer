#include "Object.hpp"

#define TINYPLY_IMPLEMENTATION

#include <tinyply.h>

#include <stdexcept>

Object::Object(RTCDevice device, RTCScene scene, const std::string &path) {
    std::ifstream in(path, std::ios::binary);

    if (in.fail()) {
        throw std::runtime_error("Failed to open " + path);
    }

    tinyply::PlyFile file;

    file.parse_header(in);
    auto vertices = file.request_properties_from_element("vertex", {"x", "y", "z"});
    auto faces = file.request_properties_from_element("face", {"vertex_indices"}, 3);
    file.read(in);

    create(
            device,
            scene,
            reinterpret_cast<Vertex *>(vertices->buffer.get()),
            vertices->count,
            reinterpret_cast<Face *>(faces->buffer.get()),
            faces->count
    );
}

Object::Object(
        RTCDevice device,
        RTCScene scene,
        Object::Vertex *vertices,
        size_t vertexCount,
        Object::Face *faces,
        size_t faceCount
) {
    create(device, scene, vertices, vertexCount, faces, faceCount);
}

Object::~Object() {
    rtcReleaseGeometry(m_geometry);
}

RTCGeometry Object::getGeometry() const {
    return m_geometry;
}

unsigned int Object::getGeometryID() const {
    return m_geometryID;
}

Object::Vertex *Object::getVertices() const {
    return m_vertices;
}

Object::Face *Object::getFaces() const {
    return m_faces;
}

size_t Object::getVertexCount() const {
    return m_vertexCount;
}

size_t Object::getFaceCount() const {
    return m_faceCount;
}

const Object::Box &Object::getAABB() const {
    return m_aabb;
}

void Object::create(
        RTCDevice device,
        RTCScene scene,
        Object::Vertex *vertices,
        size_t vertexCount,
        Object::Face *faces,
        size_t faceCount
) {
    m_geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    m_vertexCount = vertexCount;
    m_faceCount = faceCount;

    m_vertices = static_cast<Vertex *>(rtcSetNewGeometryBuffer(
            m_geometry,
            RTC_BUFFER_TYPE_VERTEX,
            0,
            RTC_FORMAT_FLOAT3,
            sizeof(Vertex),
            m_vertexCount
    ));

    m_faces = static_cast<Face *>(rtcSetNewGeometryBuffer(
            m_geometry,
            RTC_BUFFER_TYPE_INDEX,
            0,
            RTC_FORMAT_UINT3,
            sizeof(Face),
            m_faceCount
    ));

    rtcCommitGeometry(m_geometry);
    m_geometryID = rtcAttachGeometry(scene, m_geometry);

    std::copy(vertices, vertices + m_vertexCount, m_vertices);
    std::copy(faces, faces + m_faceCount, m_faces);

    for (size_t i = 0; i < m_vertexCount; i++) {
        auto vertex = m_vertices[i];

        m_aabb.boxMin = (glm::min)(m_aabb.boxMin, vertex);
        m_aabb.boxMax = (glm::max)(m_aabb.boxMax, vertex);
    }

    m_aabb.center = (m_aabb.boxMin + m_aabb.boxMax) * 0.5f;
    m_aabb.extent = m_aabb.boxMax - m_aabb.boxMin;
    m_aabb.minExtent = (std::min)({m_aabb.extent.x, m_aabb.extent.y, m_aabb.extent.z});
    m_aabb.maxExtent = (std::max)({m_aabb.extent.x, m_aabb.extent.y, m_aabb.extent.z});
}
