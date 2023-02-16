#include <windows.h>
#include <chrono>
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

void Camera::set_camera(Vector3 position, Quaternion rotation)
{
    position_ = position;
    rotation_ = rotation;
}

void Camera::pitch(float delta) const
{
    Quaternion qx = Quaternion::CreateFromAxisAngle(Vector3(1.f, 0.f, 0.f), delta);

    Quaternion orientation;
    (rotation_ * qx).Normalize(orientation);

    Vector3 euler = orientation.ToEuler();
    euler /= 57.2957795131f;
    if (abs(euler.z) - 180.0f <= 1e-6f) {
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
    Quaternion qy = Quaternion::CreateFromAxisAngle(Vector3(0.f, 1.f, 0.f), delta);
    (qy * rotation_).Normalize(rotation_);
}

const Matrix Camera::view() const
{
    return DirectX::XMMatrixLookAtLH(position_, position_ + direction(), Vector3(0.f, 1.f, 0.f));
    // return Matrix::CreateWorld(position_, direction(), Vector3(0.f, 1.f, 0.f));
}

// static
const Matrix Camera::proj()
{
    RECT rc;
    GetWindowRect(Game::inst()->win().window(), &rc);
    float width = float(rc.right - rc.left);
    float height = float(rc.bottom - rc.top);

    float aspect_ratio = width / height;

    constexpr float near_plane = 1.f;
    constexpr float far_plane = 1e4f;

    // default fov - 60 degree
    constexpr float fov = 60 / 57.2957795131f;
    auto vfov = static_cast<float>(2 * atan(tan(fov / 2) * (1.0 / aspect_ratio)));

    float field_of_view = aspect_ratio > 1.0f ? fov : vfov;
    // using reversed depth buffer, so far_plane and near_plane are swapped
    return Matrix::CreatePerspectiveFieldOfView(field_of_view, aspect_ratio, near_plane, far_plane);
}

const Matrix Camera::view_proj() const
{
    return view() * proj();
}

const Vector3& Camera::position() const
{
    return position_;
}

const Vector3 Camera::direction() const
{
    Vector3 forward = Vector3(0.f, 0.f, 1.f);
    Quaternion conj;
    rotation_.Conjugate(conj);
    { // forward * glm::conjugate(rotation_), but in SimpleMath
        Quaternion conj_inv;
        conj.Inverse(conj_inv);
        Vector3 quat_vector{ conj.x, conj.y, conj.z };
        Vector3 uv = quat_vector * forward;
        Vector3 uuv = quat_vector * forward;

        forward = forward + ((uv * conj.w) + uuv) * 2.f;
    } // looks simple
    forward.Normalize();
    return forward;
}

void Camera::move_forward(float delta)
{
    position_ += direction() * delta;
}

void Camera::move_right(float delta)
{
    // right vector is negative x (right-hand axis)
    Quaternion conj;
    rotation_.Conjugate(conj);
    Vector3 v(-delta, 0.f, 0.f); // right
    { // v * glm::conjugate(rotation_), but in SimpleMath
        Quaternion conj_inv;
        conj.Inverse(conj_inv);
        Vector3 quat_vector{ conj_inv.x, conj_inv.y, conj_inv.z };
        Vector3 uv = quat_vector * v;
        Vector3 uuv = quat_vector * v;

        v = v + ((uv * conj_inv.w) + uuv) * 2.f;
    } // looks simple
    position_ += v;
}

void Camera::move_up(float delta)
{
    position_ += Vector3(0.f, delta, 0.f);
}
