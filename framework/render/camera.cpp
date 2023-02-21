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

void Camera::set_camera(Vector3 position, Vector3 forward)
{
    position_ = position;
    forward_ = forward;
}

void Camera::pitch(float delta) // vertical
{
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 right = forward_.Cross(up);
    up = forward_.Cross(right);

    forward_ += up * delta;
    forward_.Normalize();
}

void Camera::yaw(float delta) // horizontal
{
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 right = up.Cross(forward_);

    forward_ += right * delta;
    forward_.Normalize();
}

void Camera::set_type(Camera::CameraType type)
{
    type_ = type;
}

const Matrix Camera::view() const
{
    return DirectX::XMMatrixLookAtRH(position_, position_ + direction(), Vector3(0.f, 1.f, 0.f));
}

const Matrix Camera::proj() const
{
    RECT rc;
    GetWindowRect(Game::inst()->win().window(), &rc);
    float width = float(rc.right - rc.left);
    float height = float(rc.bottom - rc.top);
    constexpr float near_plane = 1.f;
    constexpr float far_plane = 1e4f;

    switch (type_)
    {
        case CameraType::perspective:
        {
            float aspect_ratio = width / height;
            // default fov - 60 degree
            constexpr float fov = 60 / 57.2957795131f;
            auto vfov = static_cast<float>(2 * atan(tan(fov / 2) * (1.0 / aspect_ratio)));

            float field_of_view = aspect_ratio > 1.0f ? fov : vfov;
            // using reversed depth buffer, so far_plane and near_plane are swapped
            return Matrix::CreatePerspectiveFieldOfView(field_of_view, aspect_ratio, near_plane, far_plane);
        }
        case CameraType::orthographic:
        {
            RECT rc;
            GetWindowRect(Game::inst()->win().window(), &rc);
            return DirectX::XMMatrixOrthographicRH(width, height, near_plane, far_plane);
        }
        default:
        {
            break;
        }
    }
    return Matrix::Identity;
}

const Matrix Camera::view_proj() const
{
    return view() * proj();
}

const Vector3& Camera::position() const
{
    return position_;
}

const Vector3& Camera::direction() const
{
    return forward_;
}

void Camera::move_forward(float delta)
{
    position_ += forward_ * delta;
}

void Camera::move_right(float delta)
{
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 right = forward_.Cross(up);
    right.Normalize();

    position_ += right * delta;
}

void Camera::move_up(float delta)
{
    position_ += Vector3(0.f, delta, 0.f);
}
