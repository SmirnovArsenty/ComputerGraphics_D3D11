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

void Camera::pitch(float delta) const
{
    glm::quat qx = glm::angleAxis(delta, glm::vec3(1.f, 0.f, 0.f));

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
}

void Camera::yaw(float delta) const
{
    glm::quat qy = glm::angleAxis(delta, glm::vec3(0.f, 1.f, 0.f));
    glm::quat orientation = glm::normalize(qy * rotation_);
    rotation_ = orientation;
}

glm::mat4 Camera::view() const
{
    // absolutely the same outputs
#if 0
    glm::mat4 world_matrix = glm::inverse(glm::translate(glm::mat4(1.0), position_)
                             * glm::mat4_cast(rotation_));
    return glm::transpose(world_matrix);
#else
    glm::vec4 forward = glm::vec4(0.f, 0.f, 1.f, 1.f);
    forward = forward * glm::conjugate(rotation_);

    return glm::transpose(glm::lookAtRH(position_,
                                        position_ + glm::vec3(forward.x / forward.w, forward.y / forward.w, forward.z / forward.w),
                                        glm::vec3(0.f, 1.f, 0.f)));
#endif
}

// static
glm::mat4 Camera::proj()
{
    RECT rc;
    GetWindowRect(Game::inst()->win().window(), &rc);
    float width = float(rc.right - rc.left);
    float height = float(rc.bottom - rc.top);

    float aspect_ratio = width / height;

    constexpr float near_plane = 1.f;
    constexpr float far_plane = 1e4f;

    // default fov - 60 degree
    constexpr float fov = glm::degrees(60.f);
    auto vfov = static_cast<float>(2 * atan(tan(fov / 2) * (1.0 / aspect_ratio)));

    float field_of_view = aspect_ratio > 1.0f ? fov : vfov;
    // using reversed depth buffer, so far_plane and near_plane are swapped
    return glm::perspective(field_of_view, aspect_ratio, near_plane, far_plane);
}

glm::mat4 Camera::view_proj() const
{
    return view() * proj();
}

const glm::vec3& Camera::position() const
{
    return position_;
}

const glm::vec3 Camera::direction() const
{
    glm::vec4 forward = glm::vec4(0.f, 0.f, 1.f, 1.f);
    forward = forward * glm::conjugate(rotation_);
    return glm::normalize(glm::vec3(forward.x / forward.w, forward.y / forward.w, forward.z / forward.w));
}

void Camera::move_forward(float delta)
{
    position_ += glm::vec3(0.f, 0.f, delta) * glm::conjugate(rotation_);
}

void Camera::move_right(float delta)
{
    // right vector is negative x (right-hand axis)
    position_ += glm::vec3(-delta, 0.f, 0.f) * glm::conjugate(rotation_);
}

void Camera::move_up(float delta)
{
    position_ += glm::vec3(0.f, delta, 0.f);
}
