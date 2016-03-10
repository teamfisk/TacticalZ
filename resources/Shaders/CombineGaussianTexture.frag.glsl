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
}
