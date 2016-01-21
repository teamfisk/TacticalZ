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
	
	return RandomNumbers[randomIndex];
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
	
	float lengthA = length(vecB);
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
