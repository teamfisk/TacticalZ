#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 LightV;
uniform mat4 LightP;
uniform mat4 Bones[100];

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Tangent;
layout(location = 3) in vec3 BiTangent;
layout(location = 4) in vec2 TextureCoords;
layout(location = 5) in vec4 BoneIndices;
layout(location = 6) in vec4 BoneWeights;

out VertexData{
	vec3 Position;
	vec3 Normal;
	vec3 Tangent;
	vec3 BiTangent;
	vec2 TextureCoordinate;
	vec4 ExplosionColor;
	float ExplosionPercentageElapsed;
	vec4 PositionLightSpace;
}Output;

// N
mat4 biasMatrix = mat4(
vec4(0.5, 0.0, 0.0, 0.0),
vec4(0.0, 0.5, 0.0, 0.0),
vec4(0.0, 0.0, 0.5, 0.0),
vec4(0.5, 0.5, 0.5, 1.0)
);

void main()
{


	mat4 boneTransform = mat4(1);
	if(BoneWeights[0] > 0.0f){
	boneTransform = BoneWeights[0] * Bones[int(BoneIndices[0])]
				  + BoneWeights[1] * Bones[int(BoneIndices[1])]
				  + BoneWeights[2] * Bones[int(BoneIndices[2])]
				  + BoneWeights[3] * Bones[int(BoneIndices[3])];
	}
	
	//vec4 lightPos = LightP * LightV * M * vec4(Position, 1.0); // N
	
	gl_Position = P*V*M*boneTransform * vec4(Position, 1.0);
	//gl_Position = lightPos; // N

	Output.Position = (boneTransform * vec4(Position, 1.0)).xyz;
	Output.TextureCoordinate = TextureCoords;
	Output.Normal = vec3(M * vec4(Normal, 0.0));
	Output.Tangent = vec3(M * vec4(Tangent, 0.0));
 	Output.BiTangent = vec3(M * vec4(BiTangent, 0.0));
	Output.ExplosionColor = vec4(1.0);
	Output.ExplosionPercentageElapsed = 0.0;
	
	//Output.PositionLightSpace = lightPos; // N
	Output.PositionLightSpace = LightP * LightV * M * vec4(Position, 1.0);
}