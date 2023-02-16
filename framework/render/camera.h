#pragma once

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

class Camera
{
private:
    Vector3 position_{ 0.f, 1000.f, 0.f };
    mutable Quaternion rotation_{ Quaternion::Identity };

public:
    Camera();
    ~Camera();

    void set_camera(Vector3 position, Quaternion rotation);

    void pitch(float delta) const; // around right vector
    void yaw(float delta) const; // around up vector

    const Matrix view() const;
    static const Matrix proj();
    const Matrix view_proj() const;

    const Vector3& position() const;
    const Vector3 direction() const; // view vector

    // update camera position
    void move_forward(float delta);
    void move_right(float delta);
    void move_up(float delta);
};
