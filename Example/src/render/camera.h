#pragma once

#include <glm/glm.hpp>
#include <directxmath.h>

class Camera
{
private:
    glm::vec3 position_{ -1000.f, 0.f, 0.f };
    glm::vec3 target_{ 0.f, 0.f, 0.f };

public:
    Camera();
    ~Camera();

    void set_camera(glm::vec3 position, glm::vec3 target);

    void move_forward(float delta);
    void move_right(float delta);
    void move_up(float delta);

    void pitch(float delta); // around right vector
    void yaw(float delta); // around up vector

    glm::mat4 VP() const;

    void update();
};
