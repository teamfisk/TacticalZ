#version 430

layout (location = 0) in vec3 Position;

out VertexData{
	vec2 TextureCoordinate;
}Output;

void main()
{
	gl_Position = vec4(Position, 1.0);
	Output.TextureCoordinate = (vec2(Position) + 1) / 2;
}