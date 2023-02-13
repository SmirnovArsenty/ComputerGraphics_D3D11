#include <windows.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <cmath>
#include <sstream>

#include "core/game.h"
#include "win32/win.h"
#include "camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::set_camera(glm::vec3 position, glm::quat rotation)
{
    position_ = position;
    rotation_ = rotation;
}

void Camera::pitch(float delta)
{
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (last_time == now) {
        return;
    }

    float scaled_delta_rotation = {
        delta * std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1e4f
    };

    glm::quat qx = glm::angleAxis(scaled_delta_rotation, glm::vec3(0.f, 0.f, 1.f));

    glm::quat orientation = glm::normalize(rotation_ * qx);
    glm::vec3 euler = glm::degrees(glm::eulerAngles(orientation));
    if (glm::abs(euler.z) - 180.0f <= glm::epsilon<float>()) {
        if (euler.x <= -90.0f) {
            euler.x += 180.0f;
        }
        else if (euler.x >= 90.0f) {
            euler.x -= 180.0f;
        }
    }
    if (abs(euler.x) > 85.0f) {
        orientation = rotation_;
    }
    rotation_ = orientation;

    last_time = now;
}

void Camera::yaw(float delta)
{
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (last_time == now) {
        return;
    }

    float scaled_delta_rotation = {
        delta * std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1e4f
    };

    glm::quat qy = glm::angleAxis(scaled_delta_rotation, glm::vec3(0.f, 1.f, 0.f));
    glm::quat orientation = glm::normalize(qy * rotation_);
    rotation_ = orientation;

    last_time = now;
}

glm::mat4 Camera::view() const
{
    glm::mat4 world_matrix = glm::translate(glm::mat4(1.0), position_) *
                             glm::mat4_cast(rotation_) *
                             glm::scale(glm::mat4(1.0), glm::vec3(1.f));

    return glm::inverse(world_matrix);
}

// static
glm::mat4 Camera::proj()
{
    RECT rc;
    GetWindowRect(Game::inst()->win().get_window(), &rc);
    float width = float(rc.right - rc.left);
    float height = float(rc.bottom - rc.top);

    float aspect_ratio = width / height;

    constexpr float near_plane = 1e1f;
    constexpr float far_plane = 1e4f;

    // default fov - 60 degree
    float fov = glm::degrees(60.f);
    auto vfov = static_cast<float>(2 * atan(tan(fov / 2) * (1.0 / aspect_ratio)));

    float field_of_view = aspect_ratio > 1.0f ? fov : vfov;
    // using reversed depth buffer, so far_plane and near_plane are swapped
    return glm::perspective(field_of_view, aspect_ratio, far_plane, near_plane);
}

glm::mat4 Camera::view_proj() const
{
    return view() * proj();
}

void Camera::update()
{
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (last_time == now) {
        return;
    }

    float delta = float(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count()) * 1e6f;

    glm::vec3 delta_translation(0.f);
    glm::vec3 aligned_translation(0.f);

    if (GetKeyState('W') & 0x8000) { // move forward
        delta_translation.z += delta;
    }
    if (GetKeyState('S') & 0x8000) { // move backward
        delta_translation.z -= delta;
    }

    if (GetKeyState('D') & 0x8000) { // move right
        delta_translation.x += delta;
    }
    if (GetKeyState('A') & 0x8000) { // move left
        delta_translation.x -= delta;
    }

    if (GetKeyState(VK_SPACE) & 0x8000) // move up
    {
        aligned_translation.y += delta;
    }
    if (GetKeyState('C') & 0x8000) // move down
    {
        aligned_translation.y -= delta;
    }

    const auto v = (delta_translation * glm::conjugate(rotation_));
    position_ += v + aligned_translation;

    last_time = now;
}
