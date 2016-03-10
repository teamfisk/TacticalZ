#version 430

layout (binding = 0) uniform sampler2D Texture;
uniform int MaxMipMap;


in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

void main()
{
	vec4 result = vec4(0.0, 0.0, 0.0, 0.0);

	for(int i = 0; i < MaxMipMap; i++) {
		result += textureLod(Texture, Input.TextureCoordinate, i);
	}
	fragmentColor = result;
/*
	vec4 texel0 = textureLod(Texture, Input.TextureCoordinate, 0);
	vec4 texel1 = textureLod(Texture, Input.TextureCoordinate, 1);
	vec4 texel2 = textureLod(Texture, Input.TextureCoordinate, 2);
	vec4 texel3 = textureLod(Texture, Input.TextureCoordinate, 3);
	vec4 texel4 = textureLod(Texture, Input.TextureCoordinate, 4);

	fragmentColor = texel0 + texel1 + texel2 + texel3 + texel4;*/
	
}
