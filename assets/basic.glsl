#version 450 core

in vec2 fragCoord;
out vec4 fragColor;

uniform float iTime;

void main() {
	float r = 0.5 + 0.5*sin(iTime);
	float g = 0.5 + 0.5*cos(iTime);
	fragColor = vec4(r, g, fragCoord.x*fragCoord.y, 1.0);
}