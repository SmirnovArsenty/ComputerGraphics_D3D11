#include <windows.h>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <directxmath.h>
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

void Camera::set_camera(glm::vec3 position, glm::vec3 target)
{
    position_ = position;
    target_ = target;
}

void Camera::move_forward(float delta)
{
    glm::vec3 forward = glm::normalize(target_ - position_);

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
    target_ += glm::vec3(0.f, delta, 0.f);
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

    target_ = position_ + glm::normalize(forward +
                        up * (delta * std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1e9f));

    forward = glm::normalize(target_ - position_);
    if (forward == glm::vec3(0.f, 1.f, 0.f) || forward == glm::vec3(0.f, -1.f, 0.f))
    {
        target_ += glm::vec3(0.1f, 0.f, 0.f);
    }
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

    target_ = position_ + glm::normalize(forward +
                        right * (delta * std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1e9f));

    last_time = now;
}

glm::mat4 Camera::VP() const
{
    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 right = glm::cross(up, forward);
    up = glm::cross(forward, right);

    glm::mat4 view;
    view[0] = glm::vec4(right, 0.f); // column fill
    view[1] = glm::vec4(up, 0.f);
    view[2] = glm::vec4(forward, 0.f);
    view[3] = glm::vec4(position_, 1.f);

    RECT rc;
    GetWindowRect(Game::inst()->win().get_window(), &rc);
    float width = float(rc.right - rc.left);
    float height = float(rc.bottom - rc.top);

    width = width / height;
    height = 1.f;

    constexpr float near_ = 0.1f;
    constexpr float far_ = 10000.f;

    glm::mat4 proj = glm::identity<glm::mat4>();
    proj[0] = glm::vec4(2 * near_ / width, 0, 0, 0);
    proj[1] = glm::vec4(0, 2 * near_ / height, 0, 0);
    proj[2] = glm::vec4(0, 0, (far_ + near_) / (near_ - far_), -1);
    proj[3] = glm::vec4(0, 0, 2 * far_ * near_ / (near_ - far_), 0);

    // DirectX::XMVECTOR pos = DirectX::XMVectorSet(position_.x, position_.y, position_.z, 0.f);
    // DirectX::XMVECTOR target = DirectX::XMVectorSet(target_.x, target_.y, target_.z, 0.f);
    // DirectX::XMVECTOR dx_up = DirectX::XMVectorSet(up.x, up.y, up.z, 0.f);
    // DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, dx_up);
    // DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveLH(width, height, near_, far_);

    return view * proj;
}

void Camera::update()
{
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (last_time == now) {
        return;
    }

    float delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();

    if (!((GetKeyState('W') & 0x8000) && (GetKeyState('S') & 0x8000))) {
        if (GetKeyState('W') & 0x8000) { // move forward
            move_forward(delta);
        } else if (GetKeyState('S') & 0x8000) { // move backward
            move_forward(-delta);
        }
    }

    if (!((GetKeyState('D') & 0x8000) && (GetKeyState('A') & 0x8000))) {
        if (GetKeyState('D') & 0x8000) { // move right
            move_forward(delta);
        } else if (GetKeyState('A') & 0x8000) { // move left
            move_forward(-delta);
        }
    }

    if (GetKeyState(VK_LSHIFT) & 0x8000)
    {
        move_up(delta);
    }
    if (GetKeyState(VK_LCONTROL) & 0x8000)
    {
        move_up(-delta);
    }
    last_time = now;

    std::stringstream camera_log;
    camera_log << "Camera status:" << "\n";
    camera_log << "\t\tposition: " << position_.x << ", " << position_.y << ", " << position_.z << "\n";
    camera_log << "\t\ttarget: " << target_.x << ", " << target_.y << ", " << target_.z << "\n";
    OutputDebugString(camera_log.str().c_str());
}
