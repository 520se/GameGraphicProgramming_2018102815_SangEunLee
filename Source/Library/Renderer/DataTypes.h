#pragma once

#include "Common.h"

#define NUM_LIGHTS 2

namespace library
{
    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   SimpleVertex

      Summary:  Simple vertex structure containing a single field of the
                type XMFLOAT3
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct SimpleVertex
    {
        XMFLOAT3 Position;
        XMFLOAT2 TexCoord;
        XMFLOAT3 Normal;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   InstanceData

      Summary:  Instance data containing a per instance transformation 
                matrix
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct InstanceData
    {
        XMMATRIX Transformation;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   CBChangeOnCameraMovement

      Summary:  Constant buffer containing view matrix
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct CBChangeOnCameraMovement
    {
        XMMATRIX View;
        XMFLOAT4 CameraPosition;    // This is for shading. You may comment this out until then.
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   CBChangeOnResize

      Summary:  Constant buffer containing projection matrix
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct CBChangeOnResize
    {
        XMMATRIX Projection;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   CBChangesEveryFrame

      Summary:  Constant buffer containing world matrix
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct CBChangesEveryFrame
    {
        XMMATRIX World;
        XMFLOAT4 OutputColor;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   CBLights

      Summary:  Constant buffer containing lights' information
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct CBLights
    {
        XMFLOAT4 LightPositions[NUM_LIGHTS];
        XMFLOAT4 LightColors[NUM_LIGHTS];
    };
}