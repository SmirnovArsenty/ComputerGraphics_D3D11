#pragma once

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

class Camera
{
private:
    Vector3 position_{ 0.f, 100.f, 0.f };
    Vector3 forward_{ 1.f, 0.f, 0.f };

public:
    Camera();
    ~Camera();

    void set_camera(Vector3 position, Vector3 forward);

    void pitch(float delta); // around right vector
    void yaw(float delta); // around up vector

    const Matrix view() const;
    static const Matrix proj();
    const Matrix view_proj() const;

    const Vector3& position() const;
    const Vector3& direction() const;

    // update camera position
    void move_forward(float delta);
    void move_right(float delta);
    void move_up(float delta);
};
