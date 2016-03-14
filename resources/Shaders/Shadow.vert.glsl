#version 430

uniform mat4 PVM;

layout (location = 0) in vec3 Position;
layout (location = 4) in vec2 TextureCoords;

out VertexData{
	vec2 TextureCoordinate;
}Output;

void main()
{	
	gl_Position = PVM * vec4(Position, 1.0);
	Output.TextureCoordinate = TextureCoords;
}