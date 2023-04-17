#pragma once

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "render/resource/shader.h"
#include "render/resource/buffer.h"
#include "component/game_component.h"

#include "render/utils/sort_lib.h"

class ParticleComponent : public GameComponent
{
private:
    inline int align( int value, int alignment ) {
        return ( value + (alignment - 1) ) & ~(alignment - 1);
    }

    inline float RandomVariance( float median, float variance ) {
        float fUnitRandomValue = (float)rand() / (float)RAND_MAX;
        float fRange = variance * fUnitRandomValue;
        return median - variance + (2.0f * fRange);
    }

    struct GPUParticlePartA
    {
        Vector4 m_params[ 3 ];
    };

    struct GPUParticlePartB
    {
        Vector4 m_params[ 3 ];
    };

    struct EmitterConstantBuffer
    {
        Vector4 m_EmitterPosition;
        Vector4 m_EmitterVelocity;
        Vector4 m_PositionVariance;

        int                    m_MaxParticlesThisFrame;
        float                m_ParticleLifeSpan;
        float                m_StartSize;
        float                m_EndSize;
        
        
        float                m_VelocityVariance;
        float                m_Mass;
        int                    m_Index;
        int                    m_Streaks;

        int                    m_TextureIndex;
        int                    pads[ 3 ];
    };

    struct EmitterParams
    {
        Vector4 m_Position; // World position of the emitter
        Vector4 m_Velocity; // Velocity of each particle from the emitter
        Vector4 m_PositionVariance; // Variance in position of each particle
        int m_NumToEmit; // Number of particles to emit this frame
        float m_ParticleLifeSpan; // How long the particles should live
        float m_StartSize; // Size of particles at spawn time
        float m_EndSize; // Size of particle when they reach retirement age
        float m_Mass; // Mass of particle
        float m_VelocityVariance; // Variance in velocity of each particle
        int m_TextureIndex; // Index of the texture in the atlas
        bool m_Streaks; // Streak the particles in the direction of travel
    };

    struct PerFrameConstBuffer
    {
        Vector4 m_StartColor;
        Vector4 m_EndColor;
        Vector4 m_EmitterLightingCenter;

        Matrix m_ViewProjection;
        Matrix m_ViewProjInv;
        Matrix m_View;
        Matrix m_ViewInv;
        Matrix m_Projection;
        Matrix m_ProjectionInv;
        
        Vector4 m_EyePosition;
        Vector4 m_SunDirection;
        Vector4 m_SunColor;
        Vector4 m_AmbientColor;

        Vector4 m_SunDirectionVS;
        Vector4 pads2[ 3 ];

        float m_FrameTime;
        int m_ScreenWidth;
        int m_ScreenHeight;
        int m_FrameIndex;
        
        float m_AlphaThreshold;
        float m_CollisionThickness;
        float m_ElapsedTime;
        int m_CollisionsEnabled;

        int m_ShowSleepingParticles;
        int m_EnableSleepState;
        int pads[ 2 ];
    };

    ID3D11Buffer* m_pParticleBufferA;
    ID3D11ShaderResourceView* m_pParticleBufferA_SRV;
    ID3D11UnorderedAccessView* m_pParticleBufferA_UAV;

    ID3D11Buffer* m_pParticleBufferB;
    ID3D11UnorderedAccessView* m_pParticleBufferB_UAV;

    ID3D11Buffer* m_pViewSpaceParticlePositions;
    ID3D11ShaderResourceView* m_pViewSpaceParticlePositionsSRV;
    ID3D11UnorderedAccessView* m_pViewSpaceParticlePositionsUAV;

    ID3D11Buffer* m_pMaxRadiusBuffer;
    ID3D11ShaderResourceView* m_pMaxRadiusBufferSRV;
    ID3D11UnorderedAccessView* m_pMaxRadiusBufferUAV;

    ID3D11Buffer* m_pDeadListBuffer;
    ID3D11UnorderedAccessView* m_pDeadListUAV;

    ID3D11Buffer* m_pDeadListConstantBuffer;
    ID3D11Buffer* m_pActiveListConstantBuffer;
    
    ID3D11Buffer* m_pIndexBuffer;

    Shader m_Shader;

    Shader m_QuadShader;

    Shader m_CSSimulate;
    Shader m_CSInitDeadList;
    Shader m_CSEmit;
    Shader m_CSResetParticles;

    ID3D11Buffer* m_pEmitterConstantBuffer;

    ID3D11Buffer* m_pAliveIndexBuffer;
    ID3D11ShaderResourceView* m_pAliveIndexBufferSRV;
    ID3D11UnorderedAccessView* m_pAliveIndexBufferUAV;

    int m_NumDeadParticlesOnInit;
    int m_NumDeadParticlesAfterEmit;
    int m_NumDeadParticlesAfterSimulation;
    int m_NumActiveParticlesAfterSimulation;

    bool m_ResetSystem;

    ID3D11Buffer* m_pRenderingBuffer;
    ID3D11ShaderResourceView* m_pRenderingBufferSRV;
    ID3D11UnorderedAccessView* m_pRenderingBufferUAV;

    ID3D11Texture2D* m_pRandomTexture;
    ID3D11ShaderResourceView* m_pRandomTextureSRV;

    ID3D11Buffer* m_pIndirectDrawArgsBuffer;
    ID3D11UnorderedAccessView* m_pIndirectDrawArgsBufferUAV;

    ID3D11BlendState* m_pCompositeBlendState;

    ID3D11Buffer* m_pPerFrameConstantBuffer = nullptr;
    PerFrameConstBuffer m_GlobalConstantBuffer;

    SortLib m_SortLib;

    static const int g_maxParticles = 400 * 1024;

    Shader particle_shader_;

    void emit( int numEmitters, const EmitterParams* emitters );
    void simulate( int flags, ID3D11ShaderResourceView* depthSRV );
    void sort();

    void render_quad();
    void init_dead_list();

    void fill_random_texture();
public:
    ParticleComponent();
    ~ParticleComponent();

    void initialize() override;
    void draw() override;
    void imgui() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
};
