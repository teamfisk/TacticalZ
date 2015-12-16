#version 430

#define TILE_SIZE 16
#define NUM_TILES 3600

uniform mat4 P;
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
	Frustum Data[3600];
} Frustums;

vec4 ConvertToView(vec4 ScreenCoords)
{
	vec2 normalizedScreenCoords = ScreenCoords.xy / ScreenDimensions;
	vec4 clipSpace = vec4(  vec2(normalizedScreenCoords.x, normalizedScreenCoords.y) * 2.0 - 1.0, ScreenCoords.z, ScreenCoords.w);
 	vec4 view = inverse(P) * clipSpace;					
 	view = view / view.w;
 	return view;
}

Plane ComputePlane( vec3 p0, vec3 p1, vec3 p2 )
{
    Plane plane;
 
    vec3 v0 = p1 - p0;
    vec3 v2 = p2 - p0;
 
    plane.Normal = normalize( cross( v0, v2 ) );
    plane.d = dot( vec3(plane.Normal), p0 ); // Always 0 probably
    return plane;
}

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main ()
{
	if(gl_GlobalInvocationID.x * TILE_SIZE < ScreenDimensions.x && gl_GlobalInvocationID.y * TILE_SIZE < ScreenDimensions.y) {
		//Top-Left = 0 		| Top-Right = 1 
		//Bottom-Left = 2 	| Bottom-Right = 3
		vec4 ScreenCoords[4];
		ScreenCoords[0] = vec4(gl_GlobalInvocationID.x 		 * TILE_SIZE, 	(gl_GlobalInvocationID.y + 1       ) * TILE_SIZE, -1.0, 1.0); // Z-axis might need to be 1
		ScreenCoords[1] = vec4((gl_GlobalInvocationID.x + 1) * TILE_SIZE, 	(gl_GlobalInvocationID.y + 1) * TILE_SIZE, -1.0, 1.0);
		ScreenCoords[2] = vec4(gl_GlobalInvocationID.x 		 * TILE_SIZE, 	(gl_GlobalInvocationID.y)	  * TILE_SIZE, 	-1.0, 1.0);
		ScreenCoords[3] = vec4((gl_GlobalInvocationID.x + 1) * TILE_SIZE, 	(gl_GlobalInvocationID.y) 	  * TILE_SIZE, 	-1.0, 1.0);

		vec3 ViewVectors[4];
		for(int i = 0; i < 4; i++) {
			ViewVectors[i] = vec3(ConvertToView(ScreenCoords[i]));
		}
	
		vec3 EyePos = vec3(0,0,0);

		Frustum f;
		f.Planes[0] = ComputePlane(EyePos, ViewVectors[2], ViewVectors[0]);
		f.Planes[1] = ComputePlane(EyePos, ViewVectors[1], ViewVectors[3]);
		f.Planes[2] = ComputePlane(EyePos, ViewVectors[0], ViewVectors[1]);
		f.Planes[3] = ComputePlane(EyePos, ViewVectors[3], ViewVectors[2]);



		
		if ( gl_GlobalInvocationID.x < ScreenDimensions.x / TILE_SIZE && gl_GlobalInvocationID.y < ScreenDimensions.y / TILE_SIZE ) { // innanför skärmen?
	        Frustums.Data[gl_GlobalInvocationID.x + gl_GlobalInvocationID.y*80] = f;	

	    }

		
	}
}