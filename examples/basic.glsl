#version 450 core

in vec2 fragCoord;
out vec4 fragColor;

uniform float iTime;
uniform vec2 iMouse;

void main() {
	float r = 0.5 + 0.5*sin(iTime);
	fragColor = vec4(r, fragCoord.x+iMouse.x, fragCoord.y+iMouse.y, 1.0);
}