#include <windows.h>
#include <chrono>
#include <cmath>
#include <sstream>

#include "core/game.h"
#include "win32/win.h"
#include "camera.h"
#include "render/scene/light.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

float Camera::get_near() const
{
    constexpr float near_plane = .1f;
    return near_plane;
}

float Camera::get_far() const
{
    constexpr float far_plane = 1e3f;
    return far_plane;
}

float Camera::get_fov() const
{
    RECT rc;
    GetWindowRect(Game::inst()->win().window(), &rc);
    float width = float(rc.right - rc.left);
    float height = float(rc.bottom - rc.top);
    float aspect_ratio = width / height;
    constexpr float fov = 60 / 57.2957795131f; // default fov - 60 degree
    auto vfov = static_cast<float>(2 * atan(tan(fov / 2) * (1.0 / aspect_ratio)));
    float field_of_view = aspect_ratio > 1.0f ? fov : vfov;
    return field_of_view;
}

void Camera::set_camera(Vector3 position, Vector3 forward)
{
    position_ = position;
    forward_ = forward;

    if (focus_) {
        forward_ = focus_target_ - position_;
        forward_.Normalize();
    }
}

void Camera::pitch(float delta) // vertical
{
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 right = forward_.Cross(up);
    up = forward_.Cross(right);

    if (focus_) {
        forward_ -= up * delta;
    } else {
        forward_ += up * delta;
    }
    forward_.Normalize();
    if (focus_) {
        position_ = focus_target_ - forward_ * focus_distance_;
    }
}

void Camera::yaw(float delta) // horizontal
{
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 right = up.Cross(forward_);

    if (focus_) {
        forward_ -= right * delta;
    } else {
        forward_ += right * delta;
    }
    forward_.Normalize();
    if (focus_) {
        position_ = focus_target_ - forward_ * focus_distance_;
    }
}

Camera::CameraType Camera::type() const
{
    return type_;
}

void Camera::set_type(Camera::CameraType type)
{
    type_ = type;
}

void Camera::focus(Vector3 target, float min_distance)
{
    focus_target_ = target;
    forward_ = focus_target_ - position_;
    forward_.Normalize();
    if (!focus_)
    {
        focus_min_distance_ = abs(min_distance);
        focus_distance_ = (position_ - target).Length();
        if (focus_distance_ < focus_min_distance_) {
            focus_distance_ = focus_min_distance_;
        }
    }
    position_ = focus_target_ - forward_ * focus_distance_;

    focus_ = true;
}

void Camera::reset_focus()
{
    focus_ = false;
}

const Matrix Camera::view() const
{
    Vector3 pos = position_;
    if (focus_) {
        pos = focus_target_ - forward_ * focus_distance_;
    }
    return DirectX::XMMatrixLookAtRH(pos, pos + direction(), Vector3(0.f, 1.f, 0.f));
}

const Matrix Camera::proj() const
{
    RECT rc;
    GetWindowRect(Game::inst()->win().window(), &rc);
    float width = float(rc.right - rc.left);
    float height = float(rc.bottom - rc.top);

    switch (type_)
    {
        case CameraType::perspective:
        {
            float aspect_ratio = width / height;
            return Matrix::CreatePerspectiveFieldOfView(get_fov(), aspect_ratio, get_near(), get_far());
        }
        case CameraType::orthographic:
        {
            return Matrix::CreateOrthographic(width, height, get_near(), get_far());
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

std::vector<Matrix> Camera::cascade_view_proj()
{
    std::vector<Matrix> res;
    res.reserve(Light::shadow_cascade_count);

    float iter = get_far() / Light::shadow_cascade_count;
    float near_value = get_near();
    float far_value = iter;
    for (uint32_t i = 0; i < Light::shadow_cascade_count; ++i)
    {
        RECT rc;
        GetWindowRect(Game::inst()->win().window(), &rc);
        float width = float(rc.right - rc.left);
        float height = float(rc.bottom - rc.top);
        Matrix projection = Matrix::Identity;
        switch (type_)
        {
        case CameraType::perspective:
        {
            float aspect_ratio = width / height;
            projection = Matrix::CreatePerspectiveFieldOfView(get_fov(), aspect_ratio, near_value, far_value);
        }
        case CameraType::orthographic:
        {
            projection = Matrix::CreateOrthographic(width, height, near_value, far_value);
        }
        default:
        {
            break;
        }
        }
        res.push_back(view() * projection);
        far_value += iter;
    }
    return res;
}

const Vector3& Camera::position() const
{
    return position_;
}

const Vector3& Camera::direction() const
{
    return forward_;
}

Vector3 Camera::right() const
{
    Vector3 up(0.f, 1.f, 0.f);
    return forward_.Cross(up);
}

Vector3 Camera::up() const
{
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 right = forward_.Cross(up);
    return forward_.Cross(right);
}

void Camera::move_forward(float delta)
{
    if (focus_)
    {
        focus_distance_ -= delta;
        if (focus_distance_ < focus_min_distance_) {
            focus_distance_ = focus_min_distance_;
        }
        position_ = focus_target_ - forward_ * focus_distance_;
    }
    else
    {
        position_ += forward_ * delta;
    }
}

void Camera::move_right(float delta)
{
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 right = forward_.Cross(up);
    right.Normalize();

    if (!focus_) {
        position_ += right * delta;
    }
}

void Camera::move_up(float delta)
{
    if (!focus_) {
        position_ += Vector3(0.f, delta, 0.f);
    }
}
