#include <windows.h>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <directxmath.h>

#include "core/game.h"
#include "win32/win.h"
#include "camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::set_camera(glm::vec3 position, glm::vec3 target)
{
    position_ = position;
    target_ = target;
}

void Camera::move_forward(float delta)
{
    glm::vec3 forward = glm::normalize(target_ - position_);
    // glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    // glm::vec3 right = glm::cross(up, forward);
    // up = glm::cross(forward, right);

    position_ += forward * delta;
}

void Camera::move_right(float delta)
{
    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 right = glm::cross(up, forward);
    // up = glm::cross(forward, right);

    position_ += right * delta;
}

void Camera::move_up(float delta)
{
    position_ += glm::vec3(0.f, delta, 0.f);
}

void Camera::pitch(float delta)
{
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (last_time == now) {
        return;
    }

    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 right = glm::cross(up, forward);
    up = glm::cross(forward, right);

    target_ = position_ + glm::normalize(forward + up * (delta * std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1e6f));

    last_time = now;
}

void Camera::yaw(float delta)
{
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (last_time == now) {
        return;
    }

    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 right = glm::cross(up, forward);
    up = glm::cross(forward, right);

    target_ = position_ + glm::normalize(forward + right * (delta * std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1e6f));

    last_time = now;
}

DirectX::XMMATRIX Camera::VP() const
{
    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 right = glm::cross(up, forward);
    up = glm::cross(forward, right);

    // glm::mat4 view;
    // view[0] = glm::vec4(-right, 0.f);
    // view[1] = glm::vec4(up, 0.f);
    // view[2] = glm::vec4(forward, 0.f);
    // view[3] = glm::vec4(position_, 1.f);



    RECT rc;
    GetWindowRect(Game::inst()->win().get_window(), &rc);
    float width = float(rc.right - rc.left);
    float height = float(rc.bottom - rc.top);

    constexpr float near_ = 1.f;
    constexpr float far_ = 1000.f;

    // float proj[16] = {
    //     2 * near_ / width, 0, 0, 0,
    //     0, 2 * near_ / height, 0, 0,
    //     0, 0, -(far_ + near_) / (far_ - near_), -2 * far_ * near_ / (far_ - near_),
    //     0, 0, -1, 0
    // };

    // return view * glm::mat4(glm::make_mat4(proj));
    DirectX::XMVECTOR pos = DirectX::XMVectorSet(position_.x, position_.y, position_.z, 0.f);
    DirectX::XMVECTOR target = DirectX::XMVectorSet(target_.x, target_.y, target_.z, 0.f);
    DirectX::XMVECTOR dx_up = DirectX::XMVectorSet(up.x, up.y, up.z, 0.f);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, dx_up);

    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveLH(width, height, near_, far_);

    return view * proj;
}

void Camera::update()
{
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (last_time == now) {
        return;
    }

    if (!((GetKeyState('W') & 0x8000) && (GetKeyState('S') & 0x8000))) {
        if (GetKeyState('W') & 0x8000) { // move forward
            move_forward(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1e3f);
        } else if (GetKeyState('S') & 0x8000) { // move backward
            move_forward(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / -1e3f);
        }
    }

    if (!((GetKeyState('D') & 0x8000) && (GetKeyState('A') & 0x8000))) {
        if (GetKeyState('D') & 0x8000) { // move right
            move_forward(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1e3f);
        } else if (GetKeyState('A') & 0x8000) { // move left
            move_forward(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / -1e3f);
        }
    }

    last_time = now;
}
