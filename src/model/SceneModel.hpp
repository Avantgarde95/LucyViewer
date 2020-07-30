#pragma once

#include <QObject>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "../base/Object.hpp"

class SceneModel : public QObject {
Q_OBJECT

private:
    struct Camera {
        glm::vec3 position;
        glm::vec3 center;
        glm::vec3 up;
        float fov;
    };

    struct Light {
        glm::vec3 position;
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
    };

    struct RayShoot {
        glm::vec3 position;
        glm::vec<3, glm::vec3> coefficient;
    };

public:
    explicit SceneModel(QObject *parent = nullptr);
    ~SceneModel() override;

    void render();

    const std::vector<glm::f32> &getPixels() const;
    const glm::ivec2 &getSize() const;
    float getRPS() const;

    void setSize(const glm::ivec2 &size);
    void setMainObject(const std::string &mainObjectPath);

private:
    void updateCamera();
    void updateLights();
    void updateRayShoot();
    void animateCamera();
    void animateLights();
    void computePixel(int threadIndex, int pixelX, int pixelY);

    glm::vec3 shootRayAndComputeColor(
            int threadIndex,
            RTCIntersectContext &context,
            const glm::vec3 &position,
            const glm::vec3 &direction,
            int depth = 0
    );

    bool shootRayToLightAndCheckOcclusion(
            int threadIndex,
            RTCIntersectContext &context,
            const glm::vec3 &position,
            const glm::vec3 &direction,
            const Light &light
    );

    bool objectsExist();

    std::vector<glm::f32> m_pixels = {0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<int> m_rayCounts = {0};
    float m_rps = 0.0f;
    glm::ivec2 m_size = {1, 1};

    RTCDevice m_device;
    RTCScene m_scene;
    Object *m_mainObject = nullptr;
    Object *m_roomObject = nullptr;
    Object *m_mirrorObject = nullptr;
    glm::vec3 m_mainColor = {0.8f, 0.8f, 1.0f};
    glm::vec3 m_roomColor = {1.0f, 1.0f, 1.0f};
    glm::vec3 m_mirrorColor = {0.6f, 0.6f, 0.7f};

    Camera m_camera = {
            {1.5f, 1.5f, -1.5f},
            {0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            120.0f
    };

    std::vector<Light> m_lights = {
            {
                    {1.5f, 1.5f, -1.5f},
                    {0.3f, 0.3f, 0.3f},
                    {1.0f, 1.0f, 1.0f},
                    {1.0f, 1.0f, 1.0f}
            }
    };

    RayShoot m_rayShoot = {
            glm::vec3(0.0f),
            {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f)}
    };
};
