#version 430

uniform vec2 PickingColor;

in VertexData{
	vec3 Position;
}Input;

out vec4 TextureFragment;

void main()
{
	TextureFragment = vec4(PickingColor/255, 0, 1);
}


