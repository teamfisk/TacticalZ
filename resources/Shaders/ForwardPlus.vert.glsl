#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

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
}Output;

void main()
{
	gl_Position = P*V*M * vec4(Position, 1.0);
	mat4 TIM = transpose(inverse(M));
	Output.Position = Position;
	Output.TextureCoordinate = TextureCoords;
	Output.Normal = vec3(TIM) * Normal;
	Output.Tangent = vec3(TIM) * Tangent;
 	Output.BiTangent = vec3(TIM) * BiTangent;
	Output.ExplosionColor = vec4(1.0);
	Output.ExplosionPercentageElapsed = 0.0;
}