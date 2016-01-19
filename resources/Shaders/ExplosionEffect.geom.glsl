#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 ExplosionOrigin;
uniform float TimeSinceDeath; 
uniform float ExplosionDuration; 
//uniform bool Gravity;
//uniform float GravityForce;
//uniform float ObjectRadius;
uniform vec4 EndColor;
uniform bool Randomness;
uniform float RandomNumbers[50];
uniform float RandomnessScalar;
uniform vec2 Velocity;
uniform bool ColorByDistance;
//uniform bool ReverseAnimation;
//uniform bool Wireframe;
uniform bool ExponentialAccelaration;

in VertexData{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 DiffuseColor;
	vec4 ExplosionColor;
}Input[];

out VertexData{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 DiffuseColor;
	vec4 ExplosionColor;
}Output;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// returns a "random" number based on input parameter
float GetRandomNumber(int polygon_index)
{
	int randomIndex = int(mod(polygon_index, 50));
	
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
		case 20: return RandomNumbers[20];
		case 21: return RandomNumbers[21];
		case 22: return RandomNumbers[22];
		case 23: return RandomNumbers[23];
		case 24: return RandomNumbers[24];
		case 25: return RandomNumbers[25];
		case 26: return RandomNumbers[26];
		case 27: return RandomNumbers[27];
		case 28: return RandomNumbers[28];
		case 29: return RandomNumbers[29];
		case 30: return RandomNumbers[30];
		case 31: return RandomNumbers[31];
		case 32: return RandomNumbers[32];
		case 33: return RandomNumbers[33];
		case 34: return RandomNumbers[34];
		case 35: return RandomNumbers[35];
		case 36: return RandomNumbers[36];
		case 37: return RandomNumbers[37];
		case 38: return RandomNumbers[38];
		case 39: return RandomNumbers[39];
		case 40: return RandomNumbers[40];
		case 41: return RandomNumbers[41];
		case 42: return RandomNumbers[42];
		case 43: return RandomNumbers[43];
		case 44: return RandomNumbers[44];
		case 45: return RandomNumbers[45];
		case 46: return RandomNumbers[46];
		case 47: return RandomNumbers[47];
		case 48: return RandomNumbers[48];
		case 49: return RandomNumbers[49];
	}
}

float randomNumber = 0.0;
float randomDistance = 0.0;

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
	vec3 origin2TriangleCenterVector = centerOfTriangle - ExplosionOrigin;
	vec3 normalizedOrigin2TriangleCenterVector = normalize(origin2TriangleCenterVector);
	
	// time percentage until end of explosion (0.0-1.0)
	float timePercetage = TimeSinceDeath / ExplosionDuration;
	
		// get a random number if randomness is enabled, otherwise the random number will be zero and won't affect the other algorithms
	if (Randomness == true)
	{
		randomDistance = GetRandomNumber(gl_PrimitiveIDIn) * RandomnessScalar;
	}
	
	vec2 randomVelocity = Velocity * (randomDistance + 1.0);
	
	// accelaration to use on current frame. is a interpolation between start and end value
	float currentVelocity = mix(randomVelocity.x, randomVelocity.y, timePercetage);
	
	if (ExponentialAccelaration == true)
	{
		currentVelocity = pow(currentVelocity, 2) / 2.0;
	}
	
	// distance between origin and the center of the current triangle
	float origin2TriangleCenterDistance = length(origin2TriangleCenterVector);
	
	// the distance from origin to the triangle center with eventual randomness added
	float fullDistanceWithRandomness = origin2TriangleCenterDistance + randomDistance;

	// vector from the triangle center to the explosion's shockwave
	vec3 triangleCenter2ExplosionRadius = (normalizedOrigin2TriangleCenterVector * (TimeSinceDeath * currentVelocity)) - (normalizedOrigin2TriangleCenterVector * fullDistanceWithRandomness);
	
	// if the triangle is inside the blast radius...
	if (fullDistanceWithRandomness <= (TimeSinceDeath * currentVelocity))
	{
		float maxRadius = max(TimeSinceDeath * currentVelocity, (ExplosionDuration * currentVelocity));
		
		float a = (randomVelocity.y - randomVelocity.x) / ExplosionDuration;
		//float t = sqrt((2 * origin2TriangleCenterDistance) / a);
		
		// if explosion color should be affected by distance instead of time...
		if (ColorByDistance == true)
		{
			// calculate the max distance (s) the triangle will move
			float s = (randomVelocity.x * ExplosionDuration) + (0.5 * a * pow(ExplosionDuration, 2));
			
			Output.ExplosionColor = EndColor * (length(triangleCenter2ExplosionRadius) / s);
		}
		else
		{
			Output.ExplosionColor = EndColor * timePercetage;
		}
		
		// for every vertex on the triangle...
		for (int i = 0; i < gl_in.length(); i++)
		{
			// move the triangle to the blast radius
			vec3 ExplodedPosition = Input[i].Position + triangleCenter2ExplosionRadius;
			
			// pass through vertex data
			Output.Normal = Input[i].Normal;
			Output.Position = Input[i].Position;
			Output.TextureCoordinate = Input[i].TextureCoordinate;
			Output.DiffuseColor = Input[i].DiffuseColor;
			
			// convert to model space for the gravity to always be in -y
			vec4 ExplodedPositionInModelSpace = M * vec4(ExplodedPosition, 1.0);
			
			// if there is gravity, apply it
			//if (Gravity == true)
			//{				
			//	// ---DO THIS----> //ExplodedPositionInModelSpace.y = ExplodedPositionInModelSpace.y - TimeSinceHit;
			//	
			//	ExplodedPositionInModelSpace.y = ExplodedPositionInModelSpace.y - pow(TimeSinceDeath, 2);
			//}
			
			// convert to screen space 
			gl_Position = P*V * ExplodedPositionInModelSpace;
			EmitVertex();
		}
	}
	else
	{
		// if explosion color should be affected by distance instead of time...
		if (ColorByDistance == true)
		{
			Output.ExplosionColor = vec4(0.0);
		}
		else
		{
			Output.ExplosionColor = EndColor * timePercetage;
		}
		
		// for every vertex on the triangle...
		for(int i = 0; i < gl_in.length(); i++)
		{
			// pass through vertex data
			Output.Normal = Input[i].Normal;
			Output.Position = Input[i].Position;
			Output.TextureCoordinate = Input[i].TextureCoordinate;
			Output.DiffuseColor = Input[i].DiffuseColor;
			
			// no change in position, pass through vertex
			gl_Position = gl_in[i].gl_Position;
			EmitVertex();
		}
	}
	
	//EndPrimitive();
}
