#version 430

uniform mat4 PVM;

layout(location = 0) in vec3 Position;
layout(location = 5) in vec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;


out VertexData{
	vec3 Position;
}Output;

void main()
{
	gl_Position = PVM * vec4(Position, 1.0);

	Output.Position = Position;
}