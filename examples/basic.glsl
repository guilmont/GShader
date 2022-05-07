#include "header.glsl"


void main() {
	float r = 0.5 + 0.5*sin(iTime);
	fragColor = vec4(r, fragCoord.x+iMouse.x, fragCoord.y+iMouse.y, 1.0);
}
