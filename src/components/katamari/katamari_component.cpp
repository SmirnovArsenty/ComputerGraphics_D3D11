#include "katamari_component.h"
#include "render/scene/scene.h"

KatamariComponent::KatamariComponent()
{

}

void KatamariComponent::initialize()
{
    scene_ = new Scene();
    scene_->initialize();

    scene_->add_model("./resources/models/Sponza_FBX/Sponza.fbx");
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
