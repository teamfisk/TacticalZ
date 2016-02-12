#version 430

uniform vec2 PickingColor;
layout (binding = 0) uniform sampler2D Texture;
uniform vec4 Color;
uniform vec4 DiffuseColor;
uniform vec2 DiffuseUVRepeat;


in VertexData{
	vec3 Position;
	vec2 TextureCoord;
}Input;

out vec4 TextureFragment;

void main()
{
	vec4 diffuseTexel = texture2D(Texture, Input.TextureCoord * DiffuseUVRepeat);

	if(Color.a * diffuseTexel.a >= 0.9) {
		TextureFragment = vec4(PickingColor/255, 0, 1);
	}

}


