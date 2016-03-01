#version 430

layout (binding = 0) uniform sampler2D DepthBuffer;
uniform vec3 ClipInfo;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out float depthLinear;
//Just for Debug, should be depthLinear
//out vec4 fragmentColor;
void main() {
	float depthSample = texture2D(DepthBuffer, Input.TextureCoordinate).r;
	depthLinear = ClipInfo[0] / (ClipInfo[1] * depthSample + ClipInfo[2]);
	//float depthLinear = (NearClip) / ( -depthSample + 1.0f);
	//fragmentColor = vec4(depthLinear, depthLinear, depthLinear, 1.0f);
}