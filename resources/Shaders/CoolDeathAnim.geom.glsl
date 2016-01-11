#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 OriginPos;
uniform float TimeSinceDeath; 
uniform float EndOfDeath; 
uniform bool Gravity;
uniform float GravityForce;
uniform float ObjectRadius;
uniform vec4 EndColor;
uniform bool UseRandomness;
uniform float RandomNumbers[10];
uniform float RandomnessScalar;

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
out vec4 ExplosionColor;

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
	
	float dist = distance(OriginPos, centerPosS);
	
	// The distance a polygon will travel under one second.
	float travelUnitS = EndOfDeath / ObjectRadius;
	
	// The distance a polygon will travel under the current frame.
	float travelUnitF = TimeSinceDeath * travelUnitS;
	
	float randomness = 0.0f;
	int randomIndex = int(mod(gl_PrimitiveIDIn, 10));
	
	// Explosion from origin
	for(int i = 0; i < gl_in.length(); i++)
	{	
		//gl_in.gl_PrimitiveIDIn;

		if (UseRandomness == true)
		{
			switch (randomIndex)
			{
				case 0: randomness = RandomNumbers[0]; break;
				case 1: randomness = RandomNumbers[1]; break;
				case 2: randomness = RandomNumbers[2]; break;
				case 3: randomness = RandomNumbers[3]; break;
				case 4: randomness = RandomNumbers[4]; break;
				case 5: randomness = RandomNumbers[5]; break;
				case 6: randomness = RandomNumbers[6]; break;
				case 7: randomness = RandomNumbers[7]; break;
				case 8: randomness = RandomNumbers[8]; break;
				case 9: randomness = RandomNumbers[9]; break;
			}
		}
		
		float fullRandomness = dist + (randomness * RandomnessScalar);
		
		vec4 screenPos = gl_in[i].gl_Position;
		
		//if (dist + fullRandomness <= travelUnitF)
		if (fullRandomness <= travelUnitF)
		{
			vec4 changedPos = M * vec4(Input[i].Position + (oriToCenterTri * travelUnitF) - (oriToCenterTri * fullRandomness), 1.0); //objectspace
			if (Gravity == true)
			{
				changedPos.y = changedPos.y - pow((travelUnitF - fullRandomness) * GravityForce, 2);
			}
			screenPos = P*V * changedPos; //sceenspace
		}

		Normal = Input[i].Normal;
		Position = Input[i].Position;
		TextureCoordinate = Input[i].TextureCoordinate;
		DiffuseColor = Input[i].DiffuseColor;
		ExplosionColor = EndColor * travelUnitF;
		
		gl_Position = screenPos;
		//gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	
}