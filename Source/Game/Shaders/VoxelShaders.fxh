//--------------------------------------------------------------------------------------
// File: VoxelShaders.fx
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#ifndef NUM_LIGHTS
#define NUM_LIGHTS 2
#endif

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation and shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement : register(b0)
{

	matrix View;
	float4 CameraPosition;

};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize

  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer CBChangeOnResize : register(b1)
{

	matrix Projection;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation, and the 
            color of the voxel
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangesEveryFrame : register(b2)
{
	matrix World;
	float4 OutputColor;
};


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights

  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbLights : register(b3)
{
	float4 LightPositions[NUM_LIGHTS];
	float4 LightColors[NUM_LIGHTS];
};


//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader, 
            instance data included
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
	row_major matrix Transform : INSTANCE_TRANSFORM;


};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
	float3 WorldPosition : WORLDPOS;

};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VSVoxel(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT) 0;
	output.Position = mul(input.Position, input.Transform);
	output.Position = mul(output.Position, World);
	output.WorldPosition = output.Position;
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Projection);
    
    
	output.TexCoord = input.TexCoord;
	output.Normal = mul(float4(input.Normal, 0.0f), input.Transform).xyz;
	output.Normal = mul(float4(output.Normal, 0.0f), World).xyz;
    
    //output.Color = OutputColor;
	return output;

}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSVoxel(PS_INPUT input) : SV_Target
{
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 ambience = float3(0.1f, 0.1f, 0.1f);
	float3 ambienceTerm = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);
	float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);
    
	for (uint i = 0; i < NUM_LIGHTS; ++i)
	{
		ambienceTerm += (ambience) * LightColors[i].xyz;
        
		float3 lightDirection = normalize(input.WorldPosition - LightPositions[i].xyz);
		float lambertianTerm = dot(normalize(input.Normal), -lightDirection);
		diffuse += max(lambertianTerm, 0.0f) * LightColors[i].xyz;
        
		float3 reflectDirection = normalize(reflect(lightDirection, input.Normal));
		specular += pow(max(dot(-viewDirection, reflectDirection), 0.0f), 20.0f) * LightColors[i].xyz;
	}
    
	
	return float4(saturate(diffuse + specular + ambience), 1.0f) * OutputColor;

}
