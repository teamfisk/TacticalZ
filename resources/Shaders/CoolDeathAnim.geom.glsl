#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 OriginPos;
uniform float TimeSinceDeath; 
uniform float EndOfDeath; 

in VertexData{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 DiffuseColor;
}Input[];

out vec3 Normal;
out vec3 Position;
out vec2 TextureCoordinate;
out vec4 DiffuseColor;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main()
{
	// surface vectors for triangle
	//vec3 v1 = Input[1].Position - Input[0].Position;
	//vec3 v2 = Input[2].Position - Input[0].Position;
	
	//vec3 v3 = Input[1].Position - OriginPos;
	//v3 = normalize(v3);
	
	// surface normal
	//vec3 normal = cross(v1, v2);
	//normal = normalize(normal);
	
	// calculate middle of a vectors
	vec3 v1 = Input[1].Position - Input[0].Position;
	vec3 v2 = Input[2].Position - Input[0].Position;
	vec3 v3 = Input[2].Position - (v1 / 2);
	vec3 v4 = Input[1].Position - (v2 / 2);
	
	// 
	//vec3 oriToTri = Input[1].Position - OriginPos;
	//v5 = normalize(v3);
	
	
	// calculate intersection
	// normalize direction
	vec3 dir1 = normalize(v1);
	vec3 dir2 = normalize(v2);
	
	vec3 vecA = Input[2].Position - Input[1].Position; //o2 - o1
	vec3 vecB = cross(dir1, dir2);
	
	mat3 matrisA = mat3(vecA, dir2, vecB);
	mat3 matrisB = mat3(vecA, dir1, vecB);
	
	float lengthA = length(cross(dir1, dir2));
	lengthA = lengthA * lengthA;

	float s = determinant(matrisA) / lengthA;
	float t = determinant(matrisB) / lengthA;
	
	// center position of triangle
	vec3 centerPosS = Input[1].Position + (s * dir1);
	//vec3 centerPosT = Input[2].Position + (t * dir2);
	
	
	// center position of triangle to origin
	vec3 oriToCenterTri = normalize(centerPosS - OriginPos);
	
	for(int i = 0; i < gl_in.length(); i++)
	{	
		vec4 screenPos = gl_in[i].gl_Position;
		
		if (distance(OriginPos, Input[0].Position) <= TimeSinceDeath)
		{
			vec4 changedPos = M * vec4(Input[i].Position  + (oriToCenterTri * TimeSinceDeath), 1.0); //objectspace
			changedPos = vec4(changedPos.x, changedPos.y - pow(TimeSinceDeath, 2), changedPos.z, changedPos.w); //objectspace
			screenPos = P*V * changedPos; //sceenspace
		}
		// Explosion from origin
		
		

		Normal = Input[i].Normal;
		Position = Input[i].Position;
		TextureCoordinate = Input[i].TextureCoordinate;
		DiffuseColor = vec4(Input[i].DiffuseColor.rg, Input[i].DiffuseColor.b + TimeSinceDeath, Input[i].DiffuseColor.a);
		
		gl_Position = screenPos;
		//gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	
}