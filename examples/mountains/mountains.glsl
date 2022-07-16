#include "../utils/rayMarcher.hl"
#include "../utils/camera.hl"
#include "../utils/noise.hl"

uniform vec3 cSky;
uniform vec3 cGround;
uniform vec3 cCloud;
uniform vec3 cLake;
uniform vec3 vAng;
uniform float theta; // up-down
uniform float phi; // around

#define GROUND 1
#define LAKE 2

mat2 Rotate(float angle) {
    angle *= 0.01745329252; // deg to rad
    float cs = cos(angle);
    float sn = sin(angle);
    return mat2(cs, -sn, sn, cs);
}

Object GetDist(vec3 pos) {
    Object obj;
    obj.dist = pos.y - (850.0*FBM(pos.xz/800.0, 12) + 100.0);
    obj.color = cGround;
    obj.index = GROUND;
    
    // Lake
    float dist = pos.y - 350.0; 
    dist -=  2.0*FBM(0.02*pos.xz + sin(iTime*vec2(0.1,-0.25)), 2);
    objMin(obj, dist, cLake, LAKE);

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
    const float MAX_DIST = 15000.0;
    const float SURF_DIST = 0.1;

    Object obj = RayMarch(rayOrg, rayDir, MAX_STEPS, MAX_DIST, SURF_DIST);
    vec3 pos = rayOrg + obj.dist * rayDir;
    vec3 normal = GetNormal(pos);

    // diffusive light
    vec3 lightDir = vec3(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi));
    float dif = max(0.0, dot(normal, lightDir));

    //shadow
    float dist2Light = RayMarch(pos + 3.0*normal, lightDir, MAX_STEPS, MAX_DIST, SURF_DIST).dist;
    if (dist2Light < 500.0)
        dif *= 0.05;

    uv.y+= 0.3;
    float var = 0.8*FBM(4.0*uv, 12);
    vec3 skyColor = mix(cSky - 0.2*uv.y, vec3(var), smoothstep(0.2,0.6, var));

    vec3 color = obj.color*dif;

    if (obj.index == LAKE) {
        vec3 refDir = reflect(rayDir, normal);
        Object hi = RayMarch(pos + 5.0 * normal, refDir, MAX_STEPS, MAX_DIST, SURF_DIST);
        pos +=  hi.dist * refDir;
        normal = GetNormal(pos);
        float var = dif + max(0.0, dot(normal, lightDir));
    
        if (hi.dist < 15000) 
            color = mix(color, hi.color*var, 0.7);
        else
            color = mix(color, skyColor*dif, 0.7);
    }   

    float nearFar = smoothstep(15000.0, 15000.1, obj.dist); // To avoid far field aberrationi
    color = mix(color, skyColor, nearFar);

    // Let's add some fog to the distance
    color = mix(cSky, color, exp(-0.00002 * vec3(1.0,2.0,4.0) * pow(obj.dist, 0.87))); 

    color = pow(color, vec3(0.4545)); // gamma correction
    fragColor = vec4(color, 1.0);
}
