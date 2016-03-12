#version 430

uniform mat4 PVM;
uniform mat4 Bones[100];

layout (location = 0) in vec3 Position;
layout (location = 4) in vec2 TextureCoords;
layout(location = 5) in vec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;

out VertexData{
	vec2 TextureCoordinate;
}Output;

void main()
{	
	mat4 boneTransform = mat4(1);
	if(BoneWeights[0] > 0.0f){
	boneTransform = BoneWeights[0] * Bones[int(BoneIndices[0])]
				  + BoneWeights[1] * Bones[int(BoneIndices[1])]
				  + BoneWeights[2] * Bones[int(BoneIndices[2])]
				  + BoneWeights[3] * Bones[int(BoneIndices[3])];
	}

	gl_Position = PVM * boneTransform * vec4(Position, 1.0);
	Output.TextureCoordinate = TextureCoords;
}