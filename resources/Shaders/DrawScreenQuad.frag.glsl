#version 430

layout (binding = 0) uniform sampler2D Texture;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

void main()
{
	vec4 texel = texture(Texture, Input.TextureCoordinate);

	fragmentColor = texel;
	//fragmentColor = vec4(1,0.5,0.7,1);
}


