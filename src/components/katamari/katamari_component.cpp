#define NOMINMAX
#include "katamari_component.h"
#include "core/game.h"
#include "render/render.h"
#include "render/camera.h"
#include "render/scene/scene.h"
#include "render/scene/model.h"
#include "win32/win.h"
#include "win32/input.h"

KatamariComponent::KatamariComponent()
{
}

void KatamariComponent::initialize()
{
    scene_ = new Scene();

    Light sun_light;
    sun_light.set_type(Light::Type::direction);
    sun_light.set_color(Vector3(1.f, 1.f, 1.f));
    sun_light.setup_direction(Vector3(0.f, -1.f, 0.f));
    scene_->add_light(sun_light);
    sun_light.setup_direction(Vector3(1.f, -1.f, 1.f));
    scene_->add_light(sun_light);
    scene_->initialize();

    { // setup first attached object
        attached_models_.push_back({ new Model("./resources/models/Teapot_FBX/teapot.fbx"), Vector3{}, Quaternion{} });
        attached_models_.back().model->set_position(Vector3(0.f, 0.f, 0.f));
        scene_->add_model(attached_models_.back().model);
    }

    { // debug plane
        plane_ = new Model("./resources/models/Plane_FBX/1000_plane.fbx");
        plane_->set_scale(Vector3(1000.f));
        plane_->set_rotation(Quaternion::CreateFromAxisAngle(Vector3(1.f, 0.f, 0.f), 1.57079632679f));
        scene_->add_model(plane_);
        plane_->load();
    }

    { // setup free objects
        free_models_.push_back(new Model("./resources/models/WoodenLog_FBX/WoodenLog_fbx.fbx"));
        free_models_.back()->set_position(Vector3(10.f, 0.f, 0.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/WoodenLog_FBX/WoodenLog_fbx.fbx"));
        free_models_.back()->set_position(Vector3(0.f, 0.f, 10.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/WoodenLog_FBX/WoodenLog_fbx.fbx"));
        free_models_.back()->set_position(Vector3(-10.f, 0.f, 0.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/WoodenLog_FBX/WoodenLog_fbx.fbx"));
        free_models_.back()->set_position(Vector3(0.f, 0.f, -10.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/WoodenLog_FBX/WoodenLog_fbx.fbx"));
        free_models_.back()->set_position(Vector3(20.f, 0.f, 0.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/WoodenLog_FBX/WoodenLog_fbx.fbx"));
        free_models_.back()->set_position(Vector3(20.f, 0.f, 10.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/WoodenLog_FBX/WoodenLog_fbx.fbx"));
        free_models_.back()->set_position(Vector3(15.f, 0.f, 15.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/Strawberry_FBX/Strawberry_fbx.fbx"));
        free_models_.back()->set_position(Vector3(15.f, 0.f, 10.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/GiftBox_FBX/GiftBox_fbx.fbx"));
        free_models_.back()->set_position(Vector3(30.f, 0.f, 20.f));
        scene_->add_model(free_models_.back());
        free_models_.push_back(new Model("./resources/models/Tire_FBX/Tire.fbx"));
        free_models_.back()->set_position(Vector3(30.f, 0.f, -20.f));
        free_models_.back()->set_scale(Vector3(0.02f, 0.02f, 0.02f));
        scene_->add_model(free_models_.back());
    }

    for (auto& model : attached_models_) {
        model.model->load();
        radius_a_ = model.model->radius();
        radius_b_ = model.model->radius();
        radius_t_ = 1.f;
        Vector3 pos = model.model->position();
        model.model->set_position(Vector3(pos.x, model.model->radius(), pos.z));
    }

    for (auto& model : free_models_) {
        model->load();
    }
}

void KatamariComponent::draw()
{
    scene_->draw();
}

void KatamariComponent::imgui()
{
}

void KatamariComponent::reload()
{
}

void KatamariComponent::update()
{
    scene_->update();
    for (auto& model : free_models_)
    {
        auto old_pos = model->position();
        model->set_position(Vector3(old_pos.x, (model->max().y - model->min().y) / 2, old_pos.z));
    }
    auto camera = Game::inst()->render().camera();

    const auto& keyboard = Game::inst()->win().input()->keyboard();

    float rotation_delta = Game::inst()->delta_time();
    float move_delta = Game::inst()->delta_time();
    if (keyboard.up.pressed)
    {
        Vector3 pos = attached_models_[0].model->position();
        Vector3 move_direction;
        (camera->direction() * Vector3(1.f, 0.f, 1.f)).Normalize(move_direction);
        pos += move_direction * rotation_delta * attached_models_[0].model->radius();
        attached_models_[0].model->set_position(pos);
        attached_models_[0].model->set_rotation(attached_models_[0].model->rotation() * Quaternion::CreateFromAxisAngle(camera->right(), -rotation_delta));
    }
    if (keyboard.down.pressed)
    {
        Vector3 pos = attached_models_[0].model->position();
        Vector3 move_direction;
        (camera->direction() * Vector3(1.f, 0.f, 1.f)).Normalize(move_direction);
        pos -= move_direction * rotation_delta * attached_models_[0].model->radius();
        attached_models_[0].model->set_position(pos);
        attached_models_[0].model->set_rotation(attached_models_[0].model->rotation() * Quaternion::CreateFromAxisAngle(camera->right(), rotation_delta));
    }
    if (keyboard.right.pressed)
    {
        Vector3 pos = attached_models_[0].model->position();
        Vector3 move_direction;
        (camera->right() * Vector3(1.f, 0.f, 1.f)).Normalize(move_direction);
        pos += move_direction * rotation_delta * attached_models_[0].model->radius();
        attached_models_[0].model->set_position(pos);
        attached_models_[0].model->set_rotation(attached_models_[0].model->rotation() * Quaternion::CreateFromAxisAngle(camera->direction(), rotation_delta));
    }
    if (keyboard.left.pressed)
    {
        Vector3 pos = attached_models_[0].model->position();
        Vector3 move_direction;
        (camera->right() * Vector3(1.f, 0.f, 1.f)).Normalize(move_direction);
        pos -= move_direction * rotation_delta * attached_models_[0].model->radius();
        attached_models_[0].model->set_position(pos);
        attached_models_[0].model->set_rotation(attached_models_[0].model->rotation() * Quaternion::CreateFromAxisAngle(camera->direction(), -rotation_delta));
    }

    // check collision
    float radius = 0.f;
    for (auto& attached : attached_models_) {
        radius += attached.model->radius();
    }

    Vector3 current_pos = attached_models_[0].model->position();
    int32_t attach_index = -1;
    for (uint32_t i = 0; i < free_models_.size(); ++i)
    {
        auto& model = free_models_[i];
        Vector3 diff = current_pos - model->position();
        if (diff.Length() < radius + model->radius() && radius >= model->radius())
        {
            attach_index = i;
            radius_a_ = radius_b_;
            radius_b_ = radius + model->radius();
            radius_t_ = 0.f;
            break;
        }
    }
    if (attach_index != -1) {
        attached_models_.push_back({ free_models_[attach_index], free_models_[attach_index]->position() - current_pos, attached_models_[0].model->rotation() });
        free_models_.erase(free_models_.begin() + attach_index);
    }

    for (uint32_t i = 1; i < attached_models_.size(); ++i)
    {
        auto& model = attached_models_[i];
        Vector3 pos = model.attach_position;
        Quaternion reverse_initial;
        model.initial_rotation.Inverse(reverse_initial);
        Quaternion model_rotation = reverse_initial * attached_models_[0].model->rotation();
        pos = Vector3::Transform(pos, Matrix::CreateFromQuaternion(model_rotation));
        model.model->set_position(current_pos + pos);
        model.model->set_rotation(model_rotation);
    }

    { // lerp radius after attechment
        if (radius_t_ < 1.f)
        {
            radius_t_ += Game::inst()->delta_time();
            attached_models_[0].model->set_position(
                Vector3(current_pos.x, radius_a_ * (1 - radius_t_) + radius_b_ * radius_t_, current_pos.z));
        }
    }

    camera->focus(attached_models_[0].model->position(), attached_models_[0].model->radius());
}

void KatamariComponent::destroy_resources()
{
    plane_->unload();
    delete plane_;
    plane_ = nullptr;
    for (auto& model : attached_models_) {
        model.model->unload();
        delete model.model;
        model.model = nullptr;
    }
    attached_models_.clear();
    for (auto& model : free_models_) {
        model->unload();
        delete model;
        model = nullptr;
    }
    free_models_.clear();
    scene_->destroy();
    delete scene_;
    scene_ = nullptr;
}
