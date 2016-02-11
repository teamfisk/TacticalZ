#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 4) in vec2 TextureCoords;

out VertexData{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoordinate;
}Output;

void main()
{

	gl_Position = P * V * M * vec4(Position, 1.0);

	Output.Position = Position;
	Output.TextureCoordinate = TextureCoords;
	Output.Normal = Normal;
}