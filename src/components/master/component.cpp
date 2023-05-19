#include "component.h"
#include "render/render.h"
#include "render/camera.h"
#include "render/resource/shader.h"
#include "render/resource/buffer.h"
#include "render/scene/scene.h"
#include "render/scene/light.h"
#include "render/scene/model.h"
#include "render/scene/particle_system.h"

MasterComponent::MasterComponent()
{
}

MasterComponent::~MasterComponent()
{
}

void MasterComponent::initialize()
{
    auto& scene = Game::inst()->scene();

    lights_.push_back(new AmbientLight(Vector3(0.05f, 0.05f, 0.05f)));
    lights_.push_back(new DirectionLight(Vector3(1.f, 1.f, 0.8f), Vector3(1.f, -1.f, 1.f)));
    // lights_.push_back(new PointLight(Vector3(1.f, 1.f, 1.f), Vector3(0.f, 5.f, 0.f), 10));
    particle_system_ = new ParticleSystem(400 * 1024, Vector3(0, 2, 0));

    for (auto& l : lights_) {
        scene.add_light(l);
    }

    // scene.add_particle_system(particle_system_);

    // { // debug plane
    //     plane_ = new Model("./resources/models/Plane_FBX/1000_plane.fbx");
    //     plane_->set_scale(Vector3(10.f));
    //     plane_->set_rotation(Quaternion::CreateFromAxisAngle(Vector3(1.f, 0.f, 0.f), 1.57079632679f));
    //     scene.add_model(plane_);
    //     plane_->load();
    // }

    Game::inst()->render().camera()->set_camera(Vector3(5.f, 5.f, 5.f), Vector3(-1, -1, -1));
    Game::inst()->render().camera()->focus(Vector3(0, 0, 0), 5.f);

    init_bvh_ = std::make_unique<ComputeShader>();
    init_bvh_->set_compute_shader_from_file("./resources/shaders/voxelization/init_bvh.hlsl", "CSMain");
}

void MasterComponent::draw()
{

}

void MasterComponent::update()
{

}

void MasterComponent::reload()
{

}

void MasterComponent::imgui()
{
    // particle_system_->imgui();
}

void MasterComponent::destroy_resources()
{
    for (auto& l : lights_) {
        delete l;
        l = nullptr;
    }
    delete particle_system_;
    particle_system_ = nullptr;

    plane_->unload();
    delete plane_;
    plane_ = nullptr;
}
