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
uniform vec2 Velocity;           // CHANGED THIS TO VELOCITY REMEMBER YOU FUCKER
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

// returns a "random" number based on input parameter
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

float randomNumber = 0.0;

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
	
	// normalized center position of triangle to origin vector
	vec3 origin2TriangleCenterVector = normalize(centerOfTriangle - ExplosionOrigin);
	
	// time percentage until end of explosion (0.0-1.0)
	float timePercetage = TimeSinceDeath / ExplosionDuration;
	
	// accelaration to use on current frame. is a interpolation between start and end value
	float currentAccelaration = mix(Velocity.x, Velocity.y, timePercetage);
	
	// distance between origin and the center of the current triangle
	float origin2TriangleCenterDistance = distance(ExplosionOrigin, centerOfTriangle);
	
	// get a random number if randomness is enabled, otherwise the random number will be zero and won't affect the other algorithms
	if (Randomness == true)
	{
		randomNumber = GetRandomNumber(gl_PrimitiveIDIn);
	}
	
	// the distance from origin to the triangle center with eventual randomness added
	float fullDistanceWithRandomness = origin2TriangleCenterDistance + (randomNumber * RandomnessScalar);
	
	// vector from the triangle center to the explosion's shockwave
	vec3 triangleCenter2ExplosionRadius = (origin2TriangleCenterVector * (TimeSinceDeath * currentAccelaration)) - (origin2TriangleCenterVector * fullDistanceWithRandomness);

	// if the triangle is inside the blast radius...
	if (fullDistanceWithRandomness <= (TimeSinceDeath * currentAccelaration))
	{
		// if explosion color should be affected by distance instead of time...
		if (ColorPerPolygon == true)
		{
			// the position of the center of the triangle after it has exploded
			vec3 movedCenterOfTriangle = vec3(centerOfTriangle + triangleCenter2ExplosionRadius);
			
			// calculate the max distance (s) the triangle will move
			float a = (Velocity.y - Velocity.x) / ExplosionDuration;
			float s = (Velocity.x * ExplosionDuration) + (0.5 * a * pow(ExplosionDuration, 2));
			
			ExplosionColor = EndColor * (length(triangleCenter2ExplosionRadius) / s);
			//ExplosionColor = EndColor * (distance(ExplosionOrigin, movedCenterOfTriangle) - distance(ExplosionOrigin, centerOfTriangle))
		}
		else
		{
			ExplosionColor = EndColor * timePercetage;
		}
		
		// for every vertex on the triangle...
		for (int i = 0; i < gl_in.length(); i++)
		{
			// move the triangle to the blast radius
			vec3 ExplodedPosition = Input[i].Position + triangleCenter2ExplosionRadius;
			
			// pass through vertex data
			Normal = Input[i].Normal;
			Position = Input[i].Position;
			TextureCoordinate = Input[i].TextureCoordinate;
			DiffuseColor = Input[i].DiffuseColor;
			
			// convert to model space for the gravity to always be in -y
			vec4 ExplodedPositionInModelSpace = M * vec4(ExplodedPosition, 1.0);
			
			// if there is gravity, apply it
			if (Gravity == true)
			{
				ExplodedPositionInModelSpace.y = ExplodedPositionInModelSpace.y - pow((TimeSinceDeath - fullDistanceWithRandomness) * GravityForce, 2);
			}
			
			// convert to screen space
			gl_Position = P*V * vec4(ExplodedPosition, 1.0);
			EmitVertex();
		}
	}
	else
	{
		// if explosion color should be affected by distance instead of time...
		if (ColorPerPolygon == true)
		{
			ExplosionColor = vec4(0.0);
		}
		else
		{
			ExplosionColor = EndColor * timePercetage;
		}
		
		// for every vertex on the triangle...
		for(int i = 0; i < gl_in.length(); i++)
		{
			// pass through vertex data
			Normal = Input[i].Normal;
			Position = Input[i].Position;
			TextureCoordinate = Input[i].TextureCoordinate;
			DiffuseColor = Input[i].DiffuseColor;
			
			// no change in position, pass through vertex
			gl_Position = gl_in[i].gl_Position;
			EmitVertex();
		}
	}	
}
