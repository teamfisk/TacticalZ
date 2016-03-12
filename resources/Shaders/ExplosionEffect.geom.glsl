#version 430

#define MAX_SPLITS 4

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 ExplosionOrigin;
uniform float TimeSinceDeath; 
uniform float ExplosionDuration; 
uniform vec4 EndColor;
uniform bool Randomness;
uniform float RandomNumbers[50];
uniform float RandomnessScalar;
uniform vec2 Velocity;
uniform bool ColorByDistance;
uniform bool ExponentialAccelaration;
uniform bool Reverse;
uniform float ColorDistanceScalar;
//uniform bool Gravity;
//uniform bool Rotation;
//uniform bool MaxRadius;

in VertexData{
	vec3 Position;
	vec3 Normal;
	vec3 Tangent;
	vec3 BiTangent;
	vec2 TextureCoordinate;
	vec4 ExplosionColor;
	float ExplosionPercentageElapsed;
	vec4 PositionLightSpace[MAX_SPLITS];
}Input[];

out VertexData{
	vec3 Position;
	vec3 Normal;
	vec3 Tangent;
	vec3 BiTangent;
	vec2 TextureCoordinate;
	vec4 ExplosionColor;
	float ExplosionPercentageElapsed;
	vec4 PositionLightSpace[MAX_SPLITS];
}Output;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

float EqualTo(float x, float y) {
  return 1.0 - abs(sign(x - y));
}

float NotEqualTo(float x, float y) {
  return abs(sign(x - y));
}

float GreaterThan(float x, float y) {
  return max(sign(x - y), 0.0);
}

float LessThan(float x, float y) {
  return max(sign(y - x), 0.0);
}

float GreaterEqualTo(float x, float y) {
  return 1.0 - LessThan(x, y);
}

float LessEqualTo(float x, float y) {
  return 1.0 - GreaterThan(x, y);
}

// returns a "random" number based on input parameter
float GetRandomNumber(int polygon_index)
{
	return RandomNumbers[int(mod(polygon_index, 50))];
}

vec3 CalcCenterOfTriangle(vec3 vertex0, vec3 vertex1, vec3 vertex2)
{
	// calculate middle of vectors
	vec3 dir1 = normalize(vertex1 - vertex0);
	vec3 dir2 = normalize(vertex2 - vertex0);
	
	vec3 vecA = vertex2 - vertex1; //o2 - o1
	vec3 vecB = cross(dir1, dir2);
	
	mat3 matrisA = mat3(vecA, dir2, vecB);
	
	float lengthA = length(vecB);
	lengthA = lengthA * lengthA;
	
	float s = determinant(matrisA) / lengthA;
	
	// center position of triangle
	return vertex1 + (s * dir1);
}

void PassThingsThrough(int index)
{
	// pass through vertex data
	Output.Normal = Input[index].Normal;
	Output.Position = Input[index].Position;
	Output.TextureCoordinate = Input[index].TextureCoordinate;
	Output.Tangent = Input[index].Tangent;
	Output.BiTangent = Input[index].BiTangent;
	Output.ExplosionColor = EndColor;	
}

void main()
{
	vec3 centerOfTriangle = CalcCenterOfTriangle(Input[0].Position, Input[1].Position, Input[2].Position);
		
	// center position of triangle to origin vector
	vec3 origin2TriangleCenterVector = centerOfTriangle - ExplosionOrigin;
	vec3 normalizedOrigin2TriangleCenterVector = normalize(origin2TriangleCenterVector);
	
	// distance between origin and the center of the current triangle
	float origin2TriangleCenterDistance = length(origin2TriangleCenterVector);	
	
	// time percentage until end of explosion (0.0-1.0)
	float timePercetage = TimeSinceDeath / ExplosionDuration;
	
	// get a random number if randomness is enabled, otherwise the random distance will be zero and won't affect the other algorithms
	float randomDistance = float(Randomness) * GetRandomNumber(gl_PrimitiveIDIn) * RandomnessScalar;
	
	vec2 randomVelocity = Velocity * (randomDistance + 1.0);
	
	// accelaration to use on current frame. is a interpolation between start and end value
	float currentVelocity = mix(randomVelocity.x, randomVelocity.y, timePercetage);
	
	// if the accelaration should be exponential
	currentVelocity = currentVelocity * max(currentVelocity * float(ExponentialAccelaration), 1.0);
	
	// the distance the blast has moved since the beginning, i.e. its radius
	float blastWave = TimeSinceDeath * currentVelocity;
	
	// the distance from origin to the triangle center with eventual randomness added
	float origin2TriangleCenterDistanceWithRandomness = origin2TriangleCenterDistance + randomDistance;
	
	vec3 triangleCenter2ExplosionRadius = (normalizedOrigin2TriangleCenterVector * blastWave) - (normalizedOrigin2TriangleCenterVector * origin2TriangleCenterDistanceWithRandomness);

	// if the triangle is inside the blast's radius...
	float isInsideBlastRadius = LessEqualTo(origin2TriangleCenterDistanceWithRandomness, blastWave);
	
	// calculate the max distance (s) the triangle will move
	float accelaration = (randomVelocity.y - randomVelocity.x) / ExplosionDuration;
	float maxDistance = (randomVelocity.x * ExplosionDuration) + (0.5 * accelaration * ExplosionDuration * ExplosionDuration);
	
	// if explosion color should be affected by distance instead of time...
	if (ColorByDistance == true)
	{
		Output.ExplosionPercentageElapsed = (length(triangleCenter2ExplosionRadius) / maxDistance) * isInsideBlastRadius * ColorDistanceScalar;
	}
	else
	{
		Output.ExplosionPercentageElapsed = timePercetage;
	}
	
	vec3 ExplodedPosition;
	for (int i = 0; i < gl_in.length(); i++)
	{
		// move the triangle to the blast radius
		ExplodedPosition = Input[i].Position + (triangleCenter2ExplosionRadius * isInsideBlastRadius);

		// pass through vertex data
		PassThingsThrough(i);
			for (int j = 0; j < MAX_SPLITS; j++)
			{
				Output.PositionLightSpace[j] = Input[i].PositionLightSpace[j];
			}
			for (int j = 0; j < MAX_SPLITS; j++)
			{
				Output.PositionLightSpace[j] = Input[i].PositionLightSpace[j];
			}
		
		// convert to window space 
		gl_Position = P * V * M * vec4(ExplodedPosition, 1.0);
		EmitVertex();
	}
}