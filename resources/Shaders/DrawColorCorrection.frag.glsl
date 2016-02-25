#version 430

layout (binding = 0) uniform sampler2D SceneTexture;
layout (binding = 1) uniform sampler2D BloomTexture;
layout (binding = 2) uniform sampler2D SceneTextureLowRes;
layout (binding = 3) uniform sampler2D BloomTextureLowRes;
uniform float Exposure;
uniform float Gamma;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

void main()
{
	vec4 hdrColor = texture(SceneTexture, Input.TextureCoordinate);
	vec4 bloomColor = texture(BloomTexture, Input.TextureCoordinate);
	vec4 hdrColorLowRes = texture(SceneTextureLowRes, Input.TextureCoordinate);
	vec4 bloomColorLowRes = texture(BloomTextureLowRes, Input.TextureCoordinate);

	//hdrColor = hdrColor * SSAO;
	hdrColor += bloomColor;
	hdrColorLowRes;

	float hdrColorsum = hdrColorLowRes.r + hdrColorLowRes.g + hdrColorLowRes.b;
	//Toon mapping thingy
	vec3 result;
	if(hdrColorsum > 0.0) {
		result = vec3(1.0) - exp(-hdrColorLowRes.rgb * Exposure);
	} else {
		result = vec3(1.0) - exp(-hdrColor.rgb * Exposure);
	}

	//gamme correction
	result = pow(result, vec3(1.0 / Gamma));
	fragmentColor = vec4(result, 1.0);
	//fragmentColor = hdrColor;
	//fragmentColor = bloomColor;
	//fragmentColor = vec4(1,0.5,0.7,1);
}


