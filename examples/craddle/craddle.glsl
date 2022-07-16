#include "../utils/rayMarcher.hl"
#include "../utils/camera.hl"

uniform vec3 cBackground;
uniform vec3 cBars;
uniform vec3 cBase;
uniform vec3 cFloor;
uniform vec3 cSpheres;
uniform vec3 cWire;

mat2 Rotate(float angle) {
    angle *= 0.01745329252; // deg to rad
    float cs = cos(angle);
    float sn = sin(angle);
    return mat2(cs, -sn, sn, cs);
}

float sdfBox(vec2 pos, vec2 size, float roundCorner) {
    pos = abs(pos) - 0.5*size;
    return length(max(vec2(0.0), pos)) + min(max(pos.x, pos.y), 0.0) - roundCorner;
}

float sdfLine(vec3 pos, vec3 a, vec3 b, float thickness) {
    vec3 ap = pos - a;
    vec3 ab = b - a;

    float t = min(max(dot(ap, ab) / dot(ab, ab), 0.0), 1.0);
    vec3 c = a + t*ab;

    return distance(pos, c) - thickness;
}


Object genBalls(vec3 pos, vec3 com, float radius, float angle) {
    // wire start and end points
    vec3 a = vec3(0.0, 1.1*radius, 0.0);
    vec3 b = vec3(0.0, 0.35, 0.2);
    
    // recentering coordinates
    pos -= com;

    // rotating
    pos -= b;
    pos.xy *= Rotate(angle);
    pos += b;

    vec3 p = pos - vec3(0.0, 1.01 * radius, 0.0);
    p.yz *= Rotate(90.0);

    // creating sphere
    float sphere = sdfSphere(pos, radius);
    float ring = sdfTorus(p, 0.2*radius, 0.005);
    
    Object obj;
    obj.dist = min(sphere, ring);
    obj.color = cSpheres;

    // creating wires
    pos.z = abs(pos.z);
    float wire = sdfLine(pos, a, b, 0.003);
    objMin(obj, wire, cWire);

    return obj;
}


Object GetDist(vec3 pos) {
    Object obj;
    obj.dist = pos.y;
    obj.color = cFloor;

    // base
    objMin(obj, max(sdfBox(pos, vec3(1.0, 0.1, 0.5), 0.04), -pos.y), cBase);

    // bars
    float barHeight = 0.5;
    float barWidth = 0.2;
    float dist = length(vec2(sdfBox(pos.xy, vec2(0.7, 2.0*barHeight), 0.1), abs(pos.z)-barWidth))-0.02;
    objMin(obj, max(dist, -pos.y), cBars);

    // balls
    float radius = 0.085;
    float height = 0.25;

    float angle = 35.0 * sin(4.0 * iTime);
    float ang1 = min(angle, 0.0);
    float ang5 = max(angle, 0.0);

    float ang2 = 0.1*(angle + ang1);
    float ang3 = 0.1*angle;
    float ang4 = 0.1*(angle + ang5);
    
    vec3 com1 = vec3(+4.01*radius, height, 0.0);
    vec3 com2 = vec3(+2.01*radius, height, 0.0);
    vec3 com3 = vec3(+0.00*radius, height, 0.0);
    vec3 com4 = vec3(-2.01*radius, height, 0.0);
    vec3 com5 = vec3(-4.01*radius, height, 0.0);

    Object ball = genBalls(pos, com1, radius, ang1);
    objMin(obj, ball.dist, ball.color);

    ball = genBalls(pos, com2, radius, ang2);
    objMin(obj, ball.dist, ball.color);

    ball = genBalls(pos, com3, radius, ang3);
    objMin(obj, ball.dist, ball.color);

    ball = genBalls(pos, com4, radius, ang4);
    objMin(obj, ball.dist, ball.color);

    ball = genBalls(pos, com5, radius, ang5);
    objMin(obj, ball.dist, ball.color);

    return obj;
}

void main() {
    // moving origin to center of screen and correcting for aspect ratio
    vec2 uv = (2.0 * vec2(fragCoord.x, fragCoord.y) - 1.0) * vec2(iRatio, 1.0);
    float fov = tan(0.5*iFOV);

    // setup where we are and where we are looking
    vec3 rayOrg = iCamPos;
    float pitch = fov*uv.y + iCamPitch;
    float yaw = fov*uv.x + iCamYaw;

    vec3 rayDir;
    rayDir.x = cos(yaw)*cos(pitch);
    rayDir.y = sin(pitch);
    rayDir.z = sin(yaw)*cos(pitch);

    // Ray marcher properties
    const int MAX_STEPS = 100;
    const float MAX_DIST = 150.0;
    const float SURF_DIST = 0.001;

    Object obj = RayMarch(rayOrg, rayDir, MAX_STEPS, MAX_DIST, SURF_DIST);
    vec3 pos = rayOrg + obj.dist * rayDir;

    vec3 color = cBackground;
    if (obj.dist < MAX_DIST) {
        // diffusive light
        vec3 normal = GetNormal(pos);
        vec3 lightPos = vec3(sin(iTime), 13.0, cos(iTime));

        vec3 vLight = lightPos - pos;
        vec3 lightDir = normalize(vLight);
        float lightDist = length(vLight);
        float dif = max(0.0, dot(normal, lightDir));

        //shadow
        float dist2Light = RayMarch(pos + 0.01 * normal, lightDir, MAX_STEPS, MAX_DIST, SURF_DIST).dist;
        if (dist2Light < lightDist)
            dif *= pow(dist2Light / lightDist, 0.75);

        color = obj.color * dif;
    }
    color = pow(color, vec3(0.4545)); // gamma correction

    fragColor = vec4(color,1.0);
}
