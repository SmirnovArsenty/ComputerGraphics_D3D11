#pragma once

#include "core/game.h"
#include "render/render.h"
#include "render/resource/shader.h"

class SortLib
{
public:
    SortLib();
    virtual ~SortLib();

    HRESULT init();
    void run(unsigned int maxSize, ID3D11UnorderedAccessView* sortBufferUAV, ID3D11Buffer* itemCountBuffer);
    void release();

private:
    bool sortInitial( unsigned int maxSize );
    bool sortIncremental( unsigned int presorted, unsigned int maxSize );

private:

    ID3D11Buffer* m_pcbDispatchInfo; // constant buffer containing dispatch specific information

    Shader m_CSSortStep; // CS port of the VS/PS bitonic sort
    Shader m_CSSort512; // CS implementation to sort a number of 512 element sized arrays using a single dispatch
    Shader m_CSSortInner512; // CS implementation of the "down" pass from 512 to 1
    Shader m_CSInitArgs; // CS to write indirect args for Dispatch calls

    ID3D11Buffer* m_pIndirectSortArgsBuffer;
    ID3D11UnorderedAccessView* m_pIndirectSortArgsBufferUAV;
};