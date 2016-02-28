#version 430

#define ALPHA_CUTOFF 0.3

layout (binding = 12) uniform sampler2D DiffuseTexture;
uniform float Alpha;

in VertexData{
	vec2 TextureCoordinate;
}Input;

layout (location = 0) out float ShadowMap;

void main()
{
	vec4 diffuseTexel = texture(DiffuseTexture, Input.TextureCoordinate) * Alpha;
	
	if (diffuseTexel.a < ALPHA_CUTOFF)
	{
		discard;
	}
}


