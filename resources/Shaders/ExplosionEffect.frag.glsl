#version 430

uniform vec4 Color;
uniform sampler2D texture0;

in VertexData{
	vec3 Position;
	vec4 ViewSpacePosition;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 DiffuseColor;
	vec4 ExplosionColor;
}Input;

out vec4 fragmentColor;


void main()
{
	vec4 texel = texture2D(texture0, Input.TextureCoordinate);
	fragmentColor = texel * Input.DiffuseColor * Color + Input.ExplosionColor;
}