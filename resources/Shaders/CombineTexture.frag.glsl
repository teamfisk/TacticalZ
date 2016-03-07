#version 430

uniform sampler2D Texture0;
uniform sampler2D Texture1;


in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

void main()
{
	vec4 texel0 = texture(Texture0, Input.TextureCoordinate);
	vec4 texel1 = texture(Texture1, Input.TextureCoordinate);
	float texel1_total = (texel1.r + texel1.g + texel1.b) * texel1.a;
	texel1_total = ceil(clamp(texel1_total, 0, 1));

	vec4 final = texel0 * texel1_total + texel1 * (1 - texel1_total); 
	//vec4 final = vec4(texel1_total, texel1_total, texel1_total, 1.0);
	fragmentColor = final;
}


