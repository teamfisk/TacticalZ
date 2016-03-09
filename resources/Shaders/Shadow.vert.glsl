#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

layout (location = 0) in vec3 Position;
layout (location = 4) in vec2 TextureCoords;

out VertexData{
	vec2 TextureCoordinate;
}Output;

void main()
{	
	gl_Position = P * V * M * vec4(Position, 1.0);
	Output.TextureCoordinate = TextureCoords;
}