#version 430

#define MAX_SPLITS 4

uniform mat4 PVM;
uniform mat4 VM;
uniform mat4 TIM;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 Bones[100];
uniform mat4 LightV[MAX_SPLITS];
uniform mat4 LightP[MAX_SPLITS];

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Tangent;
layout(location = 3) in vec3 BiTangent;
layout(location = 4) in vec2 TextureCoords;
layout(location = 5) in vec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;

out VertexData{
	vec3 Position;
	vec4 ViewSpacePosition;
	vec3 Normal;
	vec3 Tangent;
	vec3 BiTangent;
	vec2 TextureCoordinate;
	vec4 ExplosionColor;
	float ExplosionPercentageElapsed;
	vec4 PositionLightSpace[MAX_SPLITS];
}Output;

void main()
{
	mat4 boneTransform = mat4(1);

	boneTransform = BoneWeights[0] * Bones[int(BoneIndices[0])]
				  + BoneWeights[1] * Bones[int(BoneIndices[1])]
				  + BoneWeights[2] * Bones[int(BoneIndices[2])]
				  + BoneWeights[3] * Bones[int(BoneIndices[3])];

	gl_Position = PVM*boneTransform * vec4(Position, 1.0);
	
	Output.Position = (boneTransform * vec4(Position, 1.0)).xyz;
	Output.ViewSpacePosition = VM * vec4(Position, 1.0);
	Output.TextureCoordinate = TextureCoords;
	Output.Normal = vec3(M * boneTransform * vec4(Normal, 0.0));
	Output.Tangent = vec3(M * vec4(Tangent, 0.0));
 	Output.BiTangent = vec3(M * vec4(BiTangent, 0.0));
	Output.ExplosionColor = vec4(1.0);
	Output.ExplosionPercentageElapsed = 0.0;
	
	for(int i = 0; i < MAX_SPLITS; i++)
	{
		Output.PositionLightSpace[i] = LightP[i] * LightV[i] * M * boneTransform * vec4(Position, 1.0);
	}
}