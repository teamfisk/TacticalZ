#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 ExplosionOrigin;
uniform float TimeSinceDeath; 
uniform float ExplosionDuration; 
uniform bool Gravity;
uniform float GravityForce;
uniform float ObjectRadius;
uniform vec4 EndColor;
uniform bool Randomness;
uniform float RandomNumbers[20];
uniform float RandomnessScalar;
uniform vec2 Accelaration;
uniform bool ColorPerPolygon;
uniform bool ReverseAnimation;
uniform bool Wireframe;

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

float GetRandomNumber(int polygon_index)
{
	int randomIndex = int(mod(polygon_index, 20));
	
	switch (randomIndex)
	{
		case 0: return RandomNumbers[0];
		case 1: return RandomNumbers[1];
		case 2: return RandomNumbers[2];
		case 3: return RandomNumbers[3];
		case 4: return RandomNumbers[4];
		case 5: return RandomNumbers[5];
		case 6: return RandomNumbers[6];
		case 7: return RandomNumbers[7];
		case 8: return RandomNumbers[8];
		case 9: return RandomNumbers[9];
		case 10: return RandomNumbers[10];
		case 11: return RandomNumbers[11];
		case 12: return RandomNumbers[12];
		case 13: return RandomNumbers[13];
		case 14: return RandomNumbers[14];
		case 15: return RandomNumbers[15];
		case 16: return RandomNumbers[16];
		case 17: return RandomNumbers[17];
		case 18: return RandomNumbers[18];
		case 19: return RandomNumbers[19];
	}
}

void main()
{
	// calculate middle of vectors
	vec3 v1 = Input[1].Position - Input[0].Position;
	vec3 v2 = Input[2].Position - Input[0].Position;
	vec3 v3 = Input[2].Position - (v1 / 2);
	vec3 v4 = Input[1].Position - (v2 / 2);
	
	// calculate intersection
	
	// normalize direction
	vec3 dir1 = normalize(v1);
	vec3 dir2 = normalize(v2);
	
	vec3 vecA = Input[2].Position - Input[1].Position; //o2 - o1
	vec3 vecB = cross(dir1, dir2);
	
	mat3 matrisA = mat3(vecA, dir2, vecB);
	
	float lengthA = length(cross(dir1, dir2));
	lengthA = lengthA * lengthA;

	float s = determinant(matrisA) / lengthA;
	
	// center position of triangle
	vec3 centerOfTriangle = Input[1].Position + (s * dir1);
	
	// center position of triangle to origin vector
	vec3 origin2TriangleCenterVector = normalize(centerOfTriangle - ExplosionOrigin);
	
	// time percentage until end of explosion
	float timePercetage = TimeSinceDeath / ExplosionDuration;
	
	float currentAccelaration = mix(Accelaration.x, Accelaration.y, timePercetage);
	
	//float TimeSinceDeath2;
	
	//if (ReverseAnimation == true)
	//{
	//	TimeSinceDeath2 = pow(TimeSinceDeath, -1);
	//}
	//else
	//{
	//	TimeSinceDeath2 = TimeSinceDeath;
	//}
	
	float origin2TriangleCenterDistance = distance(ExplosionOrigin, centerOfTriangle);
	
	
	
	
	
	
	

	
	// The distance a polygon will travel under one second.
	float travelUnitS = ExplosionDuration / ObjectRadius;
	
	// The distance a polygon will travel under the current frame.
	float travelUnitF = TimeSinceDeath * travelUnitS;
	
	
	
	


	
	
	
	
	
	
	float randomNumber = 0.0;
	
	if (Randomness == true)
	{
		randomNumber = GetRandomNumber(gl_PrimitiveIDIn);
	}
	
	float fullDistanceWithRandomness = origin2TriangleCenterDistance + (randomNumber * RandomnessScalar);
	
	vec3 ExplosionRadius = (origin2TriangleCenterVector * (TimeSinceDeath /** currentAccelaration*/)) - (origin2TriangleCenterVector * fullDistanceWithRandomness);

	vec4 screenPos;
	
	if (fullDistanceWithRandomness <= TimeSinceDeath)
	{
		for(int i = 0; i < gl_in.length(); i++)
		{
			screenPos = gl_in[i].gl_Position;
			
			vec3 ExplodedPosition = Input[i].Position + ExplosionRadius;
			
			Normal = Input[i].Normal;
			Position = Input[i].Position;
			TextureCoordinate = Input[i].TextureCoordinate;
			DiffuseColor = Input[i].DiffuseColor;
			
			ExplosionColor = EndColor * timePercetage;
			
			gl_Position = P*V*M * vec4(ExplodedPosition, 1.0);
			EmitVertex();
		}
	}
	else
	{
		for(int i = 0; i < gl_in.length(); i++)
		{
			screenPos = gl_in[i].gl_Position;
			
			Normal = Input[i].Normal;
			Position = Input[i].Position;
			TextureCoordinate = Input[i].TextureCoordinate;
			DiffuseColor = Input[i].DiffuseColor;
			
			ExplosionColor = EndColor;
			
			gl_Position = screenPos;
			EmitVertex();
		}
	}
	
	//// Explosion from origin
	//for(int i = 0; i < gl_in.length(); i++)
	//{			
	//	vec4 screenPos = gl_in[i].gl_Position;
	//	
	//	if (fullRandomness <= travelUnitF)
	//	{
	//		maXimum = (origin2TriangleCenterVector * travelUnitF) - (origin2TriangleCenterVector * fullRandomness);
	//		vec3 movedPosition = vec3(Input[i].Position + maXimum);
	//		
	//		vec4 changedPos = M * vec4(movedPosition, 1.0); //objectspace
	//		if (Gravity == true)
	//		{
	//			changedPos.y = changedPos.y - pow((travelUnitF - fullRandomness) * GravityForce, 2);
	//		}
	//		screenPos = P*V * changedPos; //sceenspace
	//	}
    //
	//	Normal = Input[i].Normal;
	//	Position = Input[i].Position;
	//	TextureCoordinate = Input[i].TextureCoordinate;
	//	DiffuseColor = Input[i].DiffuseColor;
	//	if (ColorPerPolygon == false)
	//	{
	//		ExplosionColor = EndColor * travelUnitF;
	//	}
	//	else
	//	{
	//		vec3 movedCenterPos = vec3(centerOfTriangle + maXimum);
	//		ExplosionColor = EndColor * (distance(ExplosionOrigin, movedCenterPos) - distance(ExplosionOrigin, centerOfTriangle));
	//	}
	//	
	//	gl_Position = screenPos;
	//	//gl_Position = gl_in[i].gl_Position;
	//	EmitVertex();
	//}
	
}