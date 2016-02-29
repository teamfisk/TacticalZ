#version 430

#define MAX_SPLITS 4

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 LightV[MAX_SPLITS];
uniform mat4 LightP[MAX_SPLITS];

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Tangent;
layout(location = 3) in vec3 BiTangent;
layout(location = 4) in vec2 TextureCoords;

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

// N
mat4 biasMatrix = mat4(
vec4(0.5, 0.0, 0.0, 0.0),
vec4(0.0, 0.5, 0.0, 0.0),
vec4(0.0, 0.0, 0.5, 0.0),
vec4(0.5, 0.5, 0.5, 1.0)
);

void main()
{
	gl_Position = P*V*M * vec4(Position, 1.0);
	mat4 TIM = transpose(inverse(M));
	Output.Position = Position;
	Output.TextureCoordinate = TextureCoords;
	Output.Normal = vec3(TIM * vec4(Normal, 0.0));
	Output.Tangent = vec3(TIM * vec4(Tangent, 0.0));
 	Output.BiTangent = vec3(TIM * vec4(BiTangent, 0.0));
	Output.ExplosionColor = vec4(1.0);
	Output.ExplosionPercentageElapsed = 0.0;
	
	for(int i = 0; i < MAX_SPLITS; i++)
	{
		Output.PositionLightSpace[i] = LightP[i] * LightV[i] * M * vec4(Position, 1.0);
	}
}