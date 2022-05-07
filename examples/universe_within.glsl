// This code was made by following live coding session by "The Art of Code" Youtube channel
// Part 1: https://www.youtube.com/watch?v=3CycKKJiwis
// Part 2: https://www.youtube.com/watch?v=KGJUl8Teipk

#include "utils/header.hl"

float DistLine(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p - a, ba = b - a;

    float t = clamp(dot(pa, ba)/dot(ba, ba), 0.0,1.0);
    return length(pa - ba*t);
}

float Line(vec2 p, vec2 a, vec2 b) {
    float d = DistLine(p, a, b);
    float d2 = length(a-b);
    float m = smoothstep(0.03,0.01, d);
    m *=smoothstep(1.2, 0.8, d2)+smoothstep(0.05, 0.03, abs(d2 - 0.75));
    return m;
}

float N21(vec2 p) {
    p = fract(p*vec2(233.34, 851.73));
    p+= dot(p, p+23.45);
    return fract(p.x*p.y);
}

vec2 N22(vec2 p) {
    float n = N21(p);
    return vec2(n, N21(p+n));
}

vec2 GetPos(vec2 id, vec2 offset) {
    vec2 n = N22(id+offset) * iTime;
    return offset + 0.4*sin(n);
}

float Layer(vec2 uv) {
    vec2 gv = fract(uv) - 0.5;
    vec2 id = floor(uv);

    int i = 0;
    vec2 p[9];
    for (float y = -1.0; y <= 1.0; y++)
        for (float x = -1.0; x <= 1.0; x++)
            p[i++] = GetPos(id, vec2(x, y));

    float m = 0.0;
    for (int i = 0; i < 9; i++) {
        m += Line(gv, p[4], p[i]);

        vec2 j = 20.0 * (p[i] - gv);
        float sparkle = 0.5*(1.0 + sin(5.0*iTime + 10.0*fract(p[i].x))) / dot(j,j); 
        m+= sparkle;
    }

    m += Line(gv, p[1], p[3]);
    m += Line(gv, p[1], p[5]);
    m += Line(gv, p[5], p[7]);
    m += Line(gv, p[3], p[7]);

    return m;
}

void main() {
    vec2 uv = 2.0*(fragCoord - 0.5) * vec2(iRatio, 1.0);

    float m = 0.0;

    float s = sin(0.05*iTime), c = cos(0.05*iTime);
    mat2 rot = mat2(c, -s, s, c);

    uv *= 2.0*rot;

   for (float i = 0.0; i <= 1.0; i+= 1.0/4.0) {
        float z = fract(i + 0.15*iTime);
        float size = mix(10.0, 0.5, z);

        float fade = smoothstep(0.0, 0.5, z) * smoothstep(1.0, 0.8, z);

        m+= Layer(size*uv + 20.0*i)*fade;
   }
   
    vec3 base = sin(vec3(0.345,0.456,0.675)*iTime)*0.3 + 0.6;

    fragColor = vec4(m*base, 1.0);
}
