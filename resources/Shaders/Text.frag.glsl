#version 430
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

out vec4 sceneColor;
out vec4 bloomColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    vec4 color_result = textColor * sampled;
    sceneColor = vec4(color_result.xyz, clamp(color_result.a, 0, 1));
	bloomColor = vec4(clamp(color_result.xyz - 1.0, 0, 100), 1.0);
} 