#version 430

layout (binding = 0) uniform sampler2D Texture0;
layout (binding = 1) uniform sampler2D Texture1;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

void main()
{
	vec4 texel0 = texture(Texture0, Input.TextureCoordinate);
	vec4 texel1 = texture(Texture1, Input.TextureCoordinate);

	fragmentColor = texel0 + texel1;
	//fragmentColor = vec4(1,0.5,0.7,1);
}


