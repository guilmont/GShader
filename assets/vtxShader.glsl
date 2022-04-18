#version 450 core

layout(location = 0) in vec3 vPos;
layout(location = 2) in vec2 vTexCoord;

out vec2 fragCoord;

void main() {
    fragCoord  = vTexCoord;
    gl_Position = vec4(vPos, 1.0);
}                                       