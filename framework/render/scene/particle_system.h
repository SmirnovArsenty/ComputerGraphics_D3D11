#pragma once

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "render/resource/shader.h"
#include "render/resource/buffer.h"
#include "component/game_component.h"

#include "render/utils/sort_lib.h"

class ParticleSystem : public GameComponent
{
private:
    bool reset_flag_{ true };

    uint32_t max_particles_count_;

    inline int align( int value, int alignment ) {
        return ( value + (alignment - 1) ) & ~(alignment - 1);
    }

    inline float RandomVariance( float median, float variance ) {
        float fUnitRandomValue = (float)rand() / (float)RAND_MAX;
        float fRange = variance * fUnitRandomValue;
        return median - variance + (2.0f * fRange);
    }

    struct
    {
        Matrix view;
        Matrix view_inv;
        Matrix proj;
        Matrix proj_inv;
        Matrix view_proj;
        Matrix view_proj_inv;

        Vector4 camera_position;
        Vector4 camera_direction;

        float global_time;

    } uniform_data_;
    ConstBuffer uniform_buffer_;

    struct Particle
    {
        Vector3 position;
        Vector3 prev_position;
        Vector3 velocity;
        Vector3 acceleration;

        float size;
        float sizeDelta;
        float weigth;
        float weight_delta;

        float energy;
        Vector3 color;

        Vector3 color_delta;
        float dummy;
    };

    ID3D11Buffer* particle_pool_{ nullptr }; // all particles
    ID3D11ShaderResourceView* particle_pool_SRV_{ nullptr };
    ID3D11UnorderedAccessView* particle_pool_UAV_{ nullptr };

    ID3D11Buffer* dead_list_{ nullptr }; // dead particles indices
    ID3D11UnorderedAccessView* dead_list_UAV_{ nullptr };

    ID3D11Buffer* sort_list_{ nullptr }; // pair of particle index and distance (squared) to camera
    ID3D11ShaderResourceView* sort_list_SRV_{ nullptr };
    ID3D11UnorderedAccessView* sort_list_UAV_{ nullptr };

    ID3D11Buffer* indirect_args_{ nullptr }; // args for indirect draw
    ID3D11UnorderedAccessView* indirect_args_UAV_{ nullptr };

    ID3D11Buffer* dead_list_const_buffer_{ nullptr };
    ID3D11Buffer* sort_list_const_buffer_{ nullptr };

    ID3D11BlendState* blend_state_{ nullptr };

    // compute shaders
    Shader init_dead_list_;
    Shader emit_;
    Shader simulate_;
    Shader reset_;

    Shader render_;
    Shader blit_;

    SortLib sort_lib_;

    struct EmitterParams
    {
        Vector3 origin;
        uint32_t num_to_emit;

        Vector3 velocity;
        uint32_t particle_life_span;

        float start_size;
        float end_size;
        float mass;
        float velocity_variance;
    };

    void init_dead_list();
    void emit(const EmitterParams& emitter_params);
    void simulate();
    void sort();

public:
    ParticleSystem(uint32_t max_particles, Vector3 origin);
    ~ParticleSystem();

    void initialize() override;
    void draw() override;
    void imgui() override {};
    void reload() override {};
    void update() override;
    void destroy_resources() override;
};