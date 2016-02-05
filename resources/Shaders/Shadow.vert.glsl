#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

layout(location = 0) in vec3 Position;

//out VertexData{
//	vec3 Position;
//}Output;

void main()
{	
//	Output.Position = Position;
	gl_Position = P * V * M * vec4(Position, 1.0);
}