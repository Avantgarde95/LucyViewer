#include <tbb/tbb.h>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <iostream>
#include <chrono>

#include "../base/Object.hpp"
#include "SceneModel.hpp"

#define AT_START(JOB) JOB

#define AT_END(JOB) \
do { \
    saveTheWorld(); \
    JOB \
} while (m_rps > answerOfOurLife);

typedef std::chrono::time_point<std::chrono::steady_clock> Time;

static const float answerOfOurLife = 24.5f * 1000000.0f;

static void saveTheWorld() {
    tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t(0.0005));
}

static Time getTime() {
    return std::chrono::high_resolution_clock::now();
}

static float computeDurationInSeconds(const Time &startTime, const Time &endTime) {
    return std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime).count();
}

static int sumAndClear(std::vector<int> &values) {
    int result = 0;

    for (auto &value : values) {
        result += value;
        value = 0;
    }

    return result;
}

static RTCRayHit createRay(
        const glm::vec3 &position,
        const glm::vec3 &direction,
        float tNear = 0,
        float tFar = std::numeric_limits<float>::infinity()
) {
    auto ray = RTCRayHit();

    ray.ray.org_x = position.x;
    ray.ray.org_y = position.y;
    ray.ray.org_z = position.z;
    ray.ray.tnear = tNear;

    ray.ray.dir_x = direction.x;
    ray.ray.dir_y = direction.y;
    ray.ray.dir_z = direction.z;
    ray.ray.time = 0;

    ray.ray.tfar = tFar;
    ray.ray.mask = -1;

    ray.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    ray.hit.primID = RTC_INVALID_GEOMETRY_ID;
    ray.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    return ray;
}

static size_t getThreadIndex() {
    return tbb::this_task_arena::current_thread_index();
}

static size_t getThreadCount() {
    return tbb::this_task_arena::max_concurrency();
}

SceneModel::SceneModel(QObject *parent) : QObject(parent) {
    m_device = rtcNewDevice("verbose=0");

    rtcSetDeviceErrorFunction(m_device, [](void *, RTCError code, const char *message) {
        if (code != RTC_ERROR_NONE) {
            std::stringstream ss;

            ss << "[";

            switch (code) {
                case RTC_ERROR_INVALID_ARGUMENT:
                    ss << "RTC_ERROR_INVALID_ARGUMENT";
                    break;
                case RTC_ERROR_INVALID_OPERATION:
                    ss << "RTC_ERROR_INVALID_OPERATION";
                    break;
                case RTC_ERROR_OUT_OF_MEMORY:
                    ss << "RTC_ERROR_OUT_OF_MEMORY";
                    break;
                case RTC_ERROR_UNSUPPORTED_CPU:
                    ss << "RTC_ERROR_UNSUPPORTED_CPU";
                    break;
                case RTC_ERROR_CANCELLED:
                    ss << "RTC_ERROR_CANCELLED";
                    break;
                default:
                    ss << "RTC_ERROR_UNKNOWN";
                    break;
            }

            ss << "]";

            if (message != nullptr) {
                ss << " " << message;
            }

            throw std::runtime_error(ss.str());
        }
    }, nullptr);

    m_scene = rtcNewScene(m_device);
    rtcCommitScene(m_scene);
}

SceneModel::~SceneModel() {
    rtcReleaseScene(m_scene);
    rtcReleaseDevice(m_device);
}

void SceneModel::render() {
    AT_START(
            auto startTime = getTime();
            auto threadCount = getThreadCount();

            if (m_rayCounts.size() != threadCount) {
                m_rayCounts = std::vector<int>(threadCount, 0);
            }
    );

    updateRayShoot();
    animateCamera();
    animateLights();

    int tileSizeX = 8;
    int tileSizeY = 8;
    int numTilesX = (m_size.x + tileSizeX - 1) / tileSizeX;
    int numTilesY = (m_size.y + tileSizeY - 1) / tileSizeY;

    tbb::parallel_for(tbb::blocked_range<int>(0, numTilesX * numTilesY), [&](const tbb::blocked_range<int> &range) {
        int threadIndex = static_cast<int>(getThreadIndex());

        for (int taskIndex = range.begin(); taskIndex < range.end(); taskIndex++) {
            int tileY = taskIndex / numTilesX;
            int tileX = taskIndex - tileY * numTilesX;
            int x0 = tileX * tileSizeX;
            int x1 = (std::min)(x0 + tileSizeX, m_size.x);
            int y0 = tileY * tileSizeY;
            int y1 = (std::min)(y0 + tileSizeY, m_size.y);

            for (int y = y0; y < y1; y++) {
                for (int x = x0; x < x1; x++) {
                    computePixel(threadIndex, x, y);
                }
            }
        }
    });

    int totalRayCount = sumAndClear(m_rayCounts);

    AT_END(
            auto endTime = getTime();

            if (endTime > startTime) {
                m_rps = static_cast<float>(totalRayCount) / computeDurationInSeconds(startTime, endTime);
            }
    );
}

const std::vector<glm::f32> &SceneModel::getPixels() const {
    return m_pixels;
}

const glm::ivec2 &SceneModel::getSize() const {
    return m_size;
}

float SceneModel::getRPS() const {
    return m_rps;
}

void SceneModel::setSize(const glm::ivec2 &size) {
    if (m_size == size) {
        return;
    }

    m_size = size;
    m_pixels = std::vector<glm::f32>(m_size.x * m_size.y * 4, 1.0f);
}

void SceneModel::setMainObject(const std::string &mainObjectPath) {
    delete m_mainObject;
    delete m_roomObject;
    delete m_mirrorObject;
    rtcReleaseScene(m_scene);

    m_scene = rtcNewScene(m_device);
    m_mainObject = new Object(m_device, m_scene, mainObjectPath);

    auto box = m_mainObject->getAABB();

    float roomRadius = box.maxExtent / 2.0f * 3.0f;
    glm::vec3 roomCenter = box.center;
    float roomXMin = roomCenter.x - roomRadius;
    float roomXMax = roomCenter.x + roomRadius;
    float roomYMin = roomCenter.y - roomRadius;
    float roomYMax = roomCenter.y + roomRadius;
    float roomZMin = roomCenter.z - roomRadius;
    float roomZMax = roomCenter.z + roomRadius;

    std::vector<Object::Vertex> roomVertices = {
            {roomXMin, roomYMin, roomZMin},
            {roomXMin, roomYMin, roomZMax},
            {roomXMin, roomYMax, roomZMin},
            {roomXMin, roomYMax, roomZMax},
            {roomXMax, roomYMin, roomZMin},
            {roomXMax, roomYMin, roomZMax},
            {roomXMax, roomYMax, roomZMin},
            {roomXMax, roomYMax, roomZMax}
    };

    std::vector<Object::Face> roomFaces = {
            //{2, 1, 0},
            //{2, 3, 1},
            {5, 6, 4},
            {7, 6, 5},
            {1, 4, 0},
            {5, 4, 1},
            {6, 3, 2},
            {6, 7, 3},
            {4, 2, 0},
            {4, 6, 2},
            {3, 5, 1},
            {7, 5, 3}
    };

    m_roomObject = new Object(
            m_device, m_scene,
            roomVertices.data(), roomVertices.size(),
            roomFaces.data(), roomFaces.size()
    );

    std::vector<Object::Vertex> mirrorVertices = {
            {roomXMin, roomYMin, roomZMin},
            {roomXMin, roomYMin, roomZMax},
            {roomXMin, roomYMax, roomZMin},
            {roomXMin, roomYMax, roomZMax}
    };

    std::vector<Object::Face> mirrorFaces = {
            {2, 1, 0},
            {2, 3, 1}
    };

    m_mirrorObject = new Object(
            m_device, m_scene,
            mirrorVertices.data(), mirrorVertices.size(),
            mirrorFaces.data(), mirrorFaces.size()
    );

    rtcCommitScene(m_scene);

    updateCamera();
    updateLights();
}

void SceneModel::updateCamera() {
    auto box = m_mainObject->getAABB();

    m_camera.position = box.center + glm::vec3(0.0f, box.maxExtent * 0.5f, 0.0f);
    m_camera.center = box.center;
    m_camera.up = {0.0f, 0.0f, 1.0f};
}

void SceneModel::updateLights() {
    auto box = m_mainObject->getAABB();

    m_lights[0].position = box.center + glm::vec3(0.0f, box.maxExtent * 1.0f, 0.0f);
}

void SceneModel::updateRayShoot() {
    float fovScale = 1.0f / std::tanf(0.4f * glm::radians(m_camera.fov));
    glm::vec3 cameraDirection = glm::normalize(m_camera.center - m_camera.position);
    glm::vec3 cameraU = glm::normalize(glm::cross(m_camera.up, cameraDirection));
    glm::vec3 cameraV = glm::normalize(glm::cross(cameraDirection, cameraU));

    m_rayShoot.position = m_camera.position;

    m_rayShoot.coefficient.x = cameraU;
    m_rayShoot.coefficient.y = -cameraV;
    m_rayShoot.coefficient.z = -0.5f * m_size.x * cameraU
                               + 0.5f * m_size.y * cameraV
                               + 0.5f * m_size.y * fovScale * cameraDirection;

    // Flip y.
    m_rayShoot.coefficient.z += m_rayShoot.coefficient.y * static_cast<float>(m_size.y);
    m_rayShoot.coefficient.y = -m_rayShoot.coefficient.y;
}

void SceneModel::animateCamera() {
    if (!objectsExist()) {
        return;
    }

    auto box = m_mainObject->getAABB();
    glm::mat4 matrix = glm::rotate(glm::mat4(1.0f), -0.01f, glm::vec3(0.0f, 0.0f, 1.0f));

    m_camera.position = glm::vec3(matrix * glm::vec4(m_camera.position - box.center, 1.0f)) + box.center;
}

void SceneModel::animateLights() {
    if (!objectsExist()) {
        return;
    }

    auto box = m_mainObject->getAABB();
    glm::mat4 matrix = glm::rotate(glm::mat4(1.0f), -0.03f, glm::vec3(0.0f, 0.0f, 1.0f));

    m_lights[0].position = glm::vec3(matrix * glm::vec4(m_lights[0].position - box.center, 1.0f)) + box.center;
}

void SceneModel::computePixel(int threadIndex, int pixelX, int pixelY) {
    auto context = RTCIntersectContext();
    rtcInitIntersectContext(&context);

    glm::vec3 resultColor = {0.0f, 0.0f, 0.0f};

    if (objectsExist()) {
        // 처음 발사한 광선. (카메라 -> 물체)
        resultColor = shootRayAndComputeColor(
                threadIndex,
                context,
                m_rayShoot.position,
                glm::normalize(
                        m_rayShoot.coefficient.x * static_cast<float>(pixelX)
                        + m_rayShoot.coefficient.y * static_cast<float>(pixelY)
                        + m_rayShoot.coefficient.z
                )
        );
    }

    int index = (pixelY * m_size.x + pixelX) * 4;

    m_pixels[index] = resultColor.r;
    m_pixels[index + 1] = resultColor.g;
    m_pixels[index + 2] = resultColor.b;
    m_pixels[index + 3] = 1.0f;
}

glm::vec3 SceneModel::shootRayAndComputeColor(
        int threadIndex,
        RTCIntersectContext &context,
        const glm::vec3 &position,
        const glm::vec3 &direction,
        int depth
) {
    auto ray = createRay(position, direction, 0.01f);
    rtcIntersect1(m_scene, &context, &ray);
    m_rayCounts[threadIndex]++;

    glm::vec3 rayOrigin = {ray.ray.org_x, ray.ray.org_y, ray.ray.org_z};
    glm::vec3 rayDirection = {ray.ray.dir_x, ray.ray.dir_y, ray.ray.dir_z};
    float rayLength = ray.ray.tfar;

    glm::vec3 hitPosition = rayOrigin + rayLength * rayDirection;
    auto hitID = ray.hit.geomID;

    if (hitID == RTC_INVALID_GEOMETRY_ID) {
        return {0.0f, 0.0f, 0.0f};
    }

    glm::vec3 resultColor = {0.0f, 0.0f, 0.0f};
    glm::vec3 objectColor = {1.0f, 1.0f, 1.0f};

    if (hitID == m_mainObject->getGeometryID()) {
        objectColor = m_mainColor;
    } else if (hitID == m_roomObject->getGeometryID()) {
        objectColor = m_roomColor;
    } else if (hitID == m_mirrorObject->getGeometryID()) {
        objectColor = m_mirrorColor;
    }

    glm::vec3 N = glm::normalize(glm::vec3(ray.hit.Ng_x, ray.hit.Ng_y, ray.hit.Ng_z));

    for (auto &light: m_lights) {
        // Diffuse reflection(난반사) 구현.
        glm::vec3 L = glm::normalize(light.position - hitPosition);
        float NdotL = glm::dot(N, L);
        float lambertian = (NdotL < 0.0f) ? 0.0f : NdotL;

        // 거리에 따른 빛의 감쇠 구현.
        float D = glm::distance(hitPosition, light.position) / m_mainObject->getAABB().maxExtent;
        float attenuation = 1.0f / (1.0f + D * 0.3f);

        glm::vec3 color = (light.ambientColor + lambertian * light.diffuseColor) * objectColor * attenuation;

        // 그림자 구현을 위해 물체에서 광원으로 광선을 발사한다.
        bool isOccluded = shootRayToLightAndCheckOcclusion(
                threadIndex,
                context,
                hitPosition,
                L,
                light
        );

        // 물체와 광원 사이에 다른 것이 있을 경우...
        if (isOccluded) {
            // 그 부분을 그림자로 표시한다.
            color *= 0.3f;
        }

        resultColor += color;
    }

    // 빛이 반사되는 물체일 경우...
    if (hitID == m_mirrorObject->getGeometryID() && depth < 1) {
        // 물체 위에서 광선을 발사하여 빛의 반사를 구현한다.
        glm::vec3 reflectionColor = shootRayAndComputeColor(
                threadIndex,
                context,
                hitPosition,
                glm::reflect(rayDirection, N),
                depth + 1
        );

        resultColor += 0.6f * reflectionColor;
    }

    return resultColor;
}

bool SceneModel::shootRayToLightAndCheckOcclusion(
        int threadIndex,
        RTCIntersectContext &context,
        const glm::vec3 &position,
        const glm::vec3 &direction,
        const SceneModel::Light &light
) {
    auto ray = createRay(
            position,
            direction,
            0.01f,
            glm::distance(position, light.position)
    );

    rtcOccluded1(m_scene, &context, &(ray.ray));
    m_rayCounts[threadIndex]++;

    return ray.ray.tfar < 0;
}

bool SceneModel::objectsExist() {
    return (m_mainObject != nullptr) && (m_roomObject != nullptr);
}
