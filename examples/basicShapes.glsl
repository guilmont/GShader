#version 450 core

in vec2 fragCoord;
out vec4 fragColor;

uniform float iTime;
uniform float iRatio;

uniform vec3 iCamPos;
uniform float iCamYaw;
uniform float iCamPitch;
uniform float iFOV;

mat2 Rotate(float angle) {
    angle *= 0.01745329252; // deg to rad
    float cs = cos(angle);
    float sn = sin(angle);
    return mat2(cs, -sn, sn, cs);
}

float sdfSphere(vec3 pos, float radius) {
    return length(pos) - radius;
}

float sdfCapsule(vec3 pos, float len, float radius) {
    pos.y -= len * clamp(pos.y / len, 0.0,1.0);
    return length(pos) - radius;
}

float sdfTorus(vec3 pos, float radius, float thickness) {
    float r = length(pos.xz) - radius;
    return length(vec2(r, pos.y)) - thickness;
} 

float sdfBox(vec3 pos, vec3 size, float roundCorner) {
    pos = abs(pos) - 0.5*size;
    return length(max(vec3(0.0), pos)) + min(max(pos.x, max(pos.y, pos.z)), 0.0) - roundCorner;
}

struct Object {
    vec3 color;
    float dist;
};

Object GetDist(vec3 pos);

vec3 GetNormal(vec3 pos) {
    float dp = 0.0001;
    float d = GetDist(pos).dist;
    float dx = GetDist(pos + vec3( dp, 0.0, 0.0)).dist;
    float dy = GetDist(pos + vec3(0.0,  dp, 0.0)).dist;
    float dz = GetDist(pos + vec3(0.0, 0.0,  dp)).dist;
    return vec3(dx-d, dy-d, dz-d)/dp;
}

Object RayMarch(vec3 pZero, vec3 dir) {
    const int MAX_STEPS = 100;        // maximum number of steps before stop searching
    const float MAX_DIST = 100.0;     // Sets maximum distance to look towards
    const float SURF_DIST = 0.0001;   // How close to the object it must be to return

    Object obj;
    obj.color = vec3(1.0,1.0,1.0);
    obj.dist = 0.0;
    for (int k = 0; k < MAX_STEPS; k++) {
        vec3 pos = pZero + obj.dist * dir;
        Object another = GetDist(pos);
        obj.dist += another.dist;
        obj.color = another.color;

        if (obj.dist > MAX_DIST || another.dist < SURF_DIST) {
            break;
        }
    }
    return obj;
}


void min(inout Object obj, float dist, vec3 color) {
    if (obj.dist > dist) {
        obj.dist = dist;
        obj.color = color;
    }
}

uniform vec3 Floor;
uniform vec3 Dress;

Object GetDist(vec3 pos) {
    float t =  3.0*iTime;

    vec3 
        cFloor = vec3(0.231, 0.6, 0.1),
        cWall  = vec3(0.8, 0.8, 0.8),
        cSkin  = vec3(0.525, 0.253, 0.184),
        cHat   = vec3(0.8, 0.05, 0.05),
        cDress = vec3(1.0, .718, 0.01);

    Object obj;
    obj.dist = pos.y;
    obj.color = cFloor;

    min(obj, 5.0-pos.z, cWall);

    pos.y -= 2.0*abs(sin(t));
    pos.xy *= Rotate(5.0*pow(sin(t), 3.0));

    // legs
    vec3 p = vec3(abs(pos.x), pos.yz) - vec3(0.9, 3.4, 0.0);
    p.xy *= Rotate(180.0 - 20.0 * abs(sin(t)));
    min(obj, sdfCapsule(p, 3.0, 0.5), cSkin); 

    // // body
    p = pos - vec3(0.0, 5.4, 0.0);
    float fr = 0.25*(4.0-p.y);
    cDress *= 0.2 + pow(abs(sin(10.*p.y)), 2.0);
    min(obj, sdfBox(p, vec3(2.0*fr, 4.0, 1.0*fr), 0.2), cDress);

    // arms
    vec3 p2 = vec3(abs(p.x), p.yz) - vec3(1.3, 1.0, 0.0);
    p2.xy *= Rotate(50.0 + 110.0 * abs(cos(t)));
    min(obj, sdfCapsule(p2, 3.5, 0.45), cSkin); 

    // head
    p = pos - vec3(0.0,8.8, 0.0);
    min(obj, sdfSphere(p, 1.0), cSkin);

    p2 = p - vec3(0.0,0.0, -1.0);
    min(obj, sdfSphere(p2, 0.22), cSkin);

    p2 = vec3(abs(p.x), p.yz) - vec3(0.3,0.3, -0.8);
    min(obj, sdfSphere(p2, 0.2), vec3(1.0));

    // hat
    float h = pow(abs(sin(t+1.57)), 15);
    min(obj, sdfTorus(p - vec3(0.0, 0.8 + 0.5*h, 0.0), 0.7, .2), cHat);
    min(obj, sdfTorus(p - vec3(0.0, 1.0 + 1.0*h, 0.0), 0.5, .2), cHat);
    min(obj, sdfSphere(p - vec3(0.0, 1.2 + 1.5*h, 0.0), .2), cHat);
    
    return obj;
}

void main() {
    // moving origin to center of screen and correcting for aspect ratio
    vec2 uv = (2.0 * vec2(fragCoord.x, fragCoord.y) - 1.0) * vec2(iRatio, 1.0);

    // setup where we are and where we are looking
    vec3 rayOrg = iCamPos;
    float pitch = iFOV*uv.y + iCamPitch;
    float yaw = iFOV*uv.x + iCamYaw;

    vec3 rayDir;
    rayDir.x = cos(yaw)*cos(pitch);
    rayDir.y = sin(pitch);
    rayDir.z = sin(yaw)*cos(pitch);


    Object obj = RayMarch(rayOrg, rayDir);
    vec3 pos = rayOrg + obj.dist * rayDir;
    vec3 normal = GetNormal(pos);

    // diffusive light
    vec3 lightPos = vec3(2.0*sin(iTime), 20.0, -10.0);

    vec3 vLight = lightPos - pos;
    vec3 lightDir = normalize(vLight);
    float lightDist = length(vLight);
    float dif = max(0.0, dot(normal, lightDir));

    //shadow
    float dist2Light = RayMarch(pos + 0.01 * normal, lightDir).dist;
    if (dist2Light < lightDist)
        dif *= pow(dist2Light / lightDist, 0.75);

    vec3 color = obj.color*vec3(dif);

    color = pow(color, vec3(0.4545)); // gamma correction
    fragColor = vec4(color,1.0);
}