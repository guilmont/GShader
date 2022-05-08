#include "utils/rayMarcher.hl"
#include "utils/camera.hl"

// Colors defined in config file
uniform vec3 cDress;
uniform vec3 cFloor;
uniform vec3 cHat;
uniform vec3 cSkin;
uniform vec3 cWall;

mat2 Rotate(float angle) {
    angle *= 0.01745329252; // deg to rad
    float cs = cos(angle);
    float sn = sin(angle);
    return mat2(cs, -sn, sn, cs);
}

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

    objMin(obj, 30.0 - abs(pos.x), cWall);
    objMin(obj, 25.0-abs(pos.z+20), cWall);

    pos.y -= 2.0*abs(sin(t));
    pos.xy *= Rotate(5.0*pow(sin(t), 3.0));

    // legs
    vec3 p = vec3(abs(pos.x), pos.yz) - vec3(0.9, 3.4, 0.0);
    p.xy *= Rotate(180.0 - 20.0 * abs(sin(t)));
    objMin(obj, sdfCapsule(p, 3.0, 0.5), cSkin); 

    // // body
    p = pos - vec3(0.0, 5.4, 0.0);
    float fr = 0.25*(4.0-p.y);
    cDress *= 0.2 + pow(abs(sin(10.*p.y)), 2.0);
    objMin(obj, sdfBox(p, vec3(2.0*fr, 4.0, 1.0*fr), 0.2), cDress);

    // arms
    vec3 p2 = vec3(abs(p.x), p.yz) - vec3(1.3, 1.0, 0.0);
    p2.xy *= Rotate(50.0 + 110.0 * abs(cos(t)));
    objMin(obj, sdfCapsule(p2, 3.5, 0.45), cSkin); 

    // head
    p = pos - vec3(0.0,8.8, 0.0);
    objMin(obj, sdfSphere(p, 1.0), cSkin);

    p2 = p - vec3(0.0,0.0, -1.0);
    objMin(obj, sdfSphere(p2, 0.22), cSkin);

    p2 = vec3(abs(p.x), p.yz) - vec3(0.3,0.3, -0.8);
    objMin(obj, sdfSphere(p2, 0.2), vec3(1.0));

    // hat
    float h = pow(abs(sin(t+1.57)), 15);
    objMin(obj, sdfTorus(p - vec3(0.0, 0.8 + 0.5*h, 0.0), 0.7, .2), cHat);
    objMin(obj, sdfTorus(p - vec3(0.0, 1.0 + 1.0*h, 0.0), 0.5, .2), cHat);
    objMin(obj, sdfSphere(p - vec3(0.0, 1.2 + 1.5*h, 0.0), .2), cHat);
    
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