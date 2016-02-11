#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

layout(location = 0) in vec3 Position;
layout(location = 5) in vec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;


out VertexData{
	vec3 Position;
}Output;

void main()
{
	gl_Position = P * V * M * vec4(Position, 1.0);

	Output.Position = Position;
}