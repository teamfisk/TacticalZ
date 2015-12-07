#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color;

uniform sampler2D texture0;


in VertexData{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 DiffuseColor;
}Input;


out vec4 fragmentColor;


void main()
{
	vec4 texel = texture2D(texture0, Input.TextureCoordinate);
	fragmentColor = texel * Input.DiffuseColor * Color;
}


