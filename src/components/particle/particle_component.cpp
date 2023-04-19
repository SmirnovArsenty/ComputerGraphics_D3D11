#include "particle_component.h"
#include "render/scene/scene.h"
#include "render/scene/particle_system.h"
#include "render/scene/light.h"

ParticleComponent::ParticleComponent()
{
    
    particle_system_ = new ParticleSystem(400 * 1024, Vector3(0, 0, 0));

}

void ParticleComponent::initialize()
{
    auto& scene = Game::inst()->scene();

    lights_.push_back(new AmbientLight(Vector3(0.05f, 0.05f, 0.05f)));
    scene.add_light(lights_.back());

    // like yellow sun
    lights_.push_back(new DirectionLight(Vector3(1.f, 1.f, 0.8f), Vector3(1.f, -1.f, 1.f)));
    scene.add_light(lights_.back());

    lights_.push_back(new PointLight(Vector3(1.f, 1.f, 1.f), Vector3(0.f, 5.f, 0.f), 10));
    scene.add_light(lights_.back());

    scene.initialize();
}
