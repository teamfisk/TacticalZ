#version 430



in VertexData{
	vec3 Position;
}Input;

//layout(location = 0 ) out vec4 ShadowMap;
layout(location = 0 ) out float ShadowMap;

void main()
{
	//ShadowMap = vec4(vec3(gl_FragCoord.z), 1.0);
	//ShadowMap = gl_FragCoord.z;
}


