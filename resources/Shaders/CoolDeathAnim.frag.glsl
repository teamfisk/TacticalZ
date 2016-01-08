#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color;

uniform sampler2D texture0;

in vec3 Normal;
in vec3 Position;
in vec2 TextureCoordinate;
in vec4 DiffuseColor;

out vec4 fragmentColor;


void main()
{
	vec4 texel = texture2D(texture0, TextureCoordinate);
	fragmentColor = texel * DiffuseColor * Color;
}