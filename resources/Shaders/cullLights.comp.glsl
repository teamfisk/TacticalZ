#version 430

//in uvec3 gl_NumWorkGroups;
//in uvec3 gl_WorkGroupID;
//in uvec3 gl_LocalInvocationID;
//in uvec3 gl_GlobalInvocationID;
//in uint  gl_LocalInvocationIndex;



#define NUM_LIGHTS 3
#define MAX_LIGHTS_PER_TILE 200
#define NUM_TILES 3600

struct Plane {
	vec3 Normal;
	float d;
};
struct Frustum {
	Plane Planes[4];
};

layout (std430, binding = 0) buffer FrustumBuffer
{
	Frustum Data[3600];
} Frustums;



layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main ()
{
		if(1 == 1) {
	}
}