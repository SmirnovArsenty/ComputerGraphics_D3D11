#include "katamari_component.h"
#include "render/scene/scene.h"

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

    scene_->add_model("./resources/models/Teapot_FBX/teapot.fbx");
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
}

void KatamariComponent::destroy_resources()
{
    scene_->destroy();
    delete scene_;
    scene_ = nullptr;
}
