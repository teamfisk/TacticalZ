#version 430

//in uvec3 gl_NumWorkGroups;
//in uvec3 gl_WorkGroupID;
//in uvec3 gl_LocalInvocationID;
//in uvec3 gl_GlobalInvocationID;
//in uint  gl_LocalInvocationIndex;

#define NUM_LIGHTS 3
#define MAX_LIGHTS_PER_TILE 200
#define NUM_TILES 3600

struct PointLight {
	vec4 Position;
	vec4 Color;
	float Radius;
	float Intensity;
	vec2 Pad;
};

layout (std430, binding = 0) buffer PointLightBuffer
{
	PointLight PointLights[NUM_LIGHTS];
};

struct Plane {
	vec3 Normal;
	float d;
};

struct Frustum {
	Plane Planes[4];
};

layout (std430, binding = 1) buffer FrustumBuffer
{
	Frustum Frustums[80*45];
};

layout (std430, binding = 3) buffer LightIndexBuffer
{
	float LightIndexList[MAX_LIGHTS_PER_TILE*NUM_TILES];
};

struct LightGrid
{
	int Amount;
	int Start;
	vec2 padding;
};

layout (std430, binding = 4) buffer LightGridBuffer
{
	LightGrid LightGrids[NUM_TILES];
};

layout (std430, binding = 5) buffer LightOffsetCounter
{
	int GlobalLightOffsetCounter;
};


layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main ()
{
	if(gl_LocalInvocationIndex == 0) {
		LightIndexList[int(gl_WorkGroupID.x) + int(gl_WorkGroupID.y)*80] = int(gl_WorkGroupID.x);
	}


}