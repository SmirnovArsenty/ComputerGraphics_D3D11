#pragma once

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>

class Camera
{
private:
    glm::vec3 position_{ 0.f, 1000.f, 0.f };
    // glm::vec3 target_{ 0.f, 0.f, 0.f };
    mutable glm::quat rotation_{ 1.f, 0.f, 0.f, 0.f};

public:
    Camera();
    ~Camera();

    void set_camera(glm::vec3 position, glm::quat rotation);

    void pitch(float delta) const; // around right vector
    void yaw(float delta) const; // around up vector

    glm::mat4 view() const;
    static glm::mat4 proj();
    glm::mat4 view_proj() const;

    const glm::vec3& position() const;
    const glm::vec3 direction() const; // view vector

    // update camera position
    void move_forward(float delta);
    void move_right(float delta);
    void move_up(float delta);
};
