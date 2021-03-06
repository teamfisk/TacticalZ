#version 430

//in uvec3 gl_NumWorkGroups;			//contains the number of workgroups that have been dispatched to a compute shader
//in uvec3 gl_WorkGroupID;				//contains the index of the workgroup currently being operated on by a compute shader
//in uvec3 gl_LocalInvocationID;		//contains the index of work item currently being operated on by a compute shader
//in uvec3 gl_GlobalInvocationID;		//contains the global index of work item currently being operated on by a compute shader
//in uint  gl_LocalInvocationIndex;		//contains the local linear index of work item currently being operated on by a compute shader



#define MAX_LIGHTS_PER_TILE 200
#define TILE_SIZE 16

uniform mat4 V;
uniform vec2 ScreenDimensions;

struct Plane {
	vec3 Normal;
	float d;
};
struct Frustum {
	Plane Planes[4];
};

layout (std430, binding = 0) buffer FrustumBuffer
{
	Frustum Data[];
} Frustums;

struct LightSource {
	vec4 Position;
	vec4 Direction;
	vec4 Color;
	float Radius;
	float Intensity;
	float Falloff;
	int Type;
};

layout (std430, binding = 1) buffer LightBuffer
{
	LightSource List[];
} LightSources;

struct LightGrid {
	float Start;
	float Amount;
	vec2 Padding;
};

layout (std430, binding = 2) buffer LightGridBuffer
{
	LightGrid Data[];
} LightGrids;

layout (std430, binding = 3) buffer LightOffsetBuffer
{
	int LightOffset[];
};

layout (std430, binding = 4) buffer LightIndexBuffer
{
	float LightIndex[];
};

shared int GroupLightCount;
shared int GroupLightIndexStartOffset;
shared int GroupLightIndex[MAX_LIGHTS_PER_TILE];
shared Frustum GroupFrustum;
int GroupIndex;

bool SphereInsidePlane(vec3 center, float radius, Plane plane)
{
	return dot(plane.Normal, center) - plane.d > -radius;
}

bool SphereInsideFrustrum(vec3 center, float radius, Frustum frustum/*, float zNear, float zFar*/)
{

	//Check depth here
	//if ( sphere.c.z - sphere.r > zNear || sphere.c.z + sphere.r < zFar )
    //{
    //    result = false;
    //}
 
 	for (int i =0; i < 4; i++)
 	{
 		if(! SphereInsidePlane(center, radius, frustum.Planes[i]))
 		{
 			return false;
 		}
 	}
 	return true;
}

void AppendLight(int li)
{
	int index;
	index = atomicAdd(GroupLightCount, 1);
	if( index < MAX_LIGHTS_PER_TILE )
	{ 
		GroupLightIndex[index] = int(li);
	}
}

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main ()
{
	GroupIndex = int(gl_WorkGroupID.x + (gl_WorkGroupID.y * int(ScreenDimensions.x/TILE_SIZE)));
	if(gl_LocalInvocationIndex == 0) {
		GroupLightCount = 0;
		GroupFrustum = Frustums.Data[GroupIndex];
	}

	barrier();
	memoryBarrierShared();

	for(int i = int(gl_LocalInvocationIndex); i < LightSources.List.length(); i += TILE_SIZE*TILE_SIZE) {
		LightSource light = LightSources.List[i];

		//if pointlight
		//Pos i view antagligen
		if(light.Type == 1) {
			if(SphereInsideFrustrum( vec3(V * light.Position), light.Radius, GroupFrustum)) {
				//TODO: Fix transparent and opaque list, and depth test.
				AppendLight( i );
			}
		}


		//if conelight

		//if directional
		if(light.Type == 2) {
			AppendLight( i );
		}

	}

	barrier();
	memoryBarrierShared();

	if(gl_LocalInvocationIndex == 0)
	{
		GroupLightIndexStartOffset = atomicAdd(LightOffset[0], GroupLightCount);
		LightGrids.Data[GroupIndex].Start = GroupLightIndexStartOffset;
		LightGrids.Data[GroupIndex].Amount = GroupLightCount;
	}

	barrier();
	

	for (uint i = gl_LocalInvocationIndex; i < GroupLightCount; i += TILE_SIZE * TILE_SIZE )
	{
	    LightIndex[GroupLightIndexStartOffset + i] = GroupLightIndex[i];
	}
}