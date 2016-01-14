#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Tangent;
layout(location = 3) in vec3 BiTangent;
layout(location = 4) in vec2 TextureCoords;
layout(location = 5) in vec4 DiffuseVertexColor;
layout(location = 6) in vec4 SpecularVertexColor;
layout(location = 7) in vec4 BoneIndices1;
layout(location = 8) in vec4 BoneIndices2;
layout(location = 9) in vec4 BoneWeights1;
layout(location = 10) in vec4 BoneWeights2;

out VertexData{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 DiffuseColor;
}Output;

void main()
{
	gl_Position = P * V * M * vec4(Position, 1.0);

	Output.Position = Position;
	Output.TextureCoordinate = TextureCoords;
	Output.Normal = Normal;
	Output.DiffuseColor = DiffuseVertexColor;
}