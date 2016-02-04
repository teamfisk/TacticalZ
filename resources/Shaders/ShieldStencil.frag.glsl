#version 430

in VertexData{
	vec3 Position;
}Input;


out vec4 fragmentColor;

void main()
{
	fragmentColor = vec4(0.5, 0.5, 0.5, 0.2);
}


