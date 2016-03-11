#version 430

uniform vec4 Color;
uniform vec4 FillColor;
uniform float FillPercentage;
uniform float ScaleX;
uniform float ScaleY;
uniform mat4 P;

layout (binding = 1) uniform sampler2D DiffuseTexture;
layout (binding = 2) uniform sampler2D GlowMapTexture;


in VertexData{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoordinate;
}Input;


out vec4 sceneColor;
out vec4 bloomColor;

void main()
{
	vec4 diffuseTexel = texture2D(DiffuseTexture, vec2(Input.TextureCoordinate.x*ScaleX, Input.TextureCoordinate.y*ScaleY));
	vec4 glowTexel = texture2D(GlowMapTexture, vec2(Input.TextureCoordinate.x*ScaleX, Input.TextureCoordinate.y*ScaleY));

	vec4 color_result = Color * diffuseTexel;

	float pos = ((P * vec4(Input.Position, 1)).y + 1.0)/2.0;

	if(pos <= FillPercentage) {
		color_result = FillColor*diffuseTexel.a;
	}

	sceneColor = vec4(color_result.xyz, clamp(color_result.a, 0, 1));

	//bloomColor = vec4(clamp((glowTexel.xyz*3) - 1.0, 0, 100), 1.0);
	bloomColor = vec4(max(color_result.xyz - 1.0, 0.0), clamp(color_result.a, 0, 1));
	//bloomColor = vec4(1.0, 1.0, 1.0, 0.0);
}


