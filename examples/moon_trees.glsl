#version 450 core

in vec2 fragCoord;
out vec4 fragColor;

uniform float iTime;
uniform float iRatio;

float rand21 (vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233)))*43758.5453123);
}

float noise(vec2 uv) {
    uv += 0.5;
    vec2 id = floor(uv);
    vec2 loc = smoothstep(0.0,1.0,fract(uv));

    float bl = rand21(id + vec2(0.0,0.0)),
          br = rand21(id + vec2(1.0,0.0)),
          bb = mix(bl, br, loc.x);
 

    float tl = rand21(id + vec2(0.0,1.0)),
          tr = rand21(id + vec2(1.0,1.0)),
          tt = mix(tl, tr, loc.x);


    return mix(bb, tt, loc.y);
}

float perlin(vec2 uv, float nOctaves) {
    float color = 0.0, norm = 0.0;   
    for (float k = 0.0; k < nOctaves; k++)
    {
        float a = 1.0/pow(2.0, k); 
        color +=  a * noise(uv*pow(2.0, k));
        norm += a;
    }

    return color/norm;
}

float TaperBox(vec2 p, float wb, float wt, float yb, float yt, float blur) {
    float m = smoothstep(-blur, blur, p.y-yb)*smoothstep(blur, -blur, p.y - yt);
    float w = mix(wb, wt, (p.y - yb)/(yt-yb));
    m *= smoothstep(blur, -blur, abs(p.x) - w);  
    
    return m;
}

vec4 Tree(vec2 uv, float blur) {
    vec3 cor_trunk = vec3(120.0, 40.0, 31.0)/255.0,
         cor_canopy = vec3(30.0,132.0,73.0)/255.0;


    float trunk = TaperBox(uv, 0.03,0.03,0.0,0.25, blur);
    
    float canopy = TaperBox(uv, 0.2,0.1,0.245,0.5, blur)   // canopy 1
                 + TaperBox(uv, 0.15,0.05,0.5,0.75, blur) // canopy 2
                 + TaperBox(uv, 0.1,0.0,0.75,1.0, blur);  // top

     float shadow = TaperBox(uv+vec2(0.25,0.0), 0.1,0.5,0.44,0.505, blur)
                  + TaperBox(uv-vec2(0.25,0.0), 0.1,0.5,0.705,0.755, blur)
                  + TaperBox(uv-vec2(0.2,0.0), 0.1,0.5,0.15,0.255, blur);
    
    vec3 color = trunk*cor_trunk + canopy * cor_canopy;
    color *= (1.0-shadow);

    return vec4(color, max(trunk, canopy));
}

float calcHeight(float x) { 
    return sin(0.423*x)+0.3*sin(x); 
}

vec4 Layer(vec2 uv, float blur) {
    // Adding some ground    
    float ground = smoothstep(blur, -blur, uv.y + calcHeight(uv.x));
    vec4 layer = vec4(0.95,0.95,1.0, ground);

    float id = floor(uv.x);
    float n = 2.0*fract(31415.15*sin(6462.12*id)) - 1.0;
    
    float dx = 0.3*n;
    float dy = calcHeight(id+0.5+dx)+0.05;
    vec2 pos = vec2(fract(uv.x) - 0.5 - dx, (1.0+0.2*n)*(uv.y + dy));
    
    vec4 tree = Tree(pos, blur);
        
    layer = mix(layer, tree, tree.a);
    layer.a = max(ground, tree.a);

    return  layer;
}

void main() {
    vec2 uv = (fragCoord - 0.5) * vec2(iRatio, 1.0f);

    // Clear background to black
    fragColor = vec4(0.0,0.0,0.0,0.0);

    float stars = pow(rand21(uv), 100.0),
          bright_moon = smoothstep(0.01, -0.01, length(uv-vec2(0.5,0.33)) -0.15),
          dark_moon = smoothstep(0.04, -0.01, length(uv-vec2(0.55,0.4)) -0.15);


    fragColor = mix(fragColor, vec4(stars), 1.0-bright_moon); 
    fragColor += bright_moon - mix(bright_moon, dark_moon, bright_moon);

    float oi = perlin(uv + vec2(0.0351, 0.06146)*iTime, 4.0);
    fragColor.rgb = mix(fragColor.rgb,vec3(oi), oi);
    fragColor.a = max(fragColor.a, oi);

    // Drawing layers
    for (float k = 0; k < 1.0; k+=0.1)
    {
        float scale = mix(15.0, 2.0, k);
        vec2 pos = scale*uv + vec2(0.15*iTime + k*100.0, 1.5*k);
        float blur = mix(0.01, 0.005, k);

        vec4 layer = Layer(pos, blur);
        layer.rgb *=  mix(1.0, 0.1, k);

        fragColor = mix(fragColor, layer, layer.a);

        float fog = perlin(pos, 4.0)*k*0.3;

        fragColor.rgb = (1.0 - fog)*fragColor.rgb + fog * vec3(0.9,0.9,1.0);

        if (fragColor.a < 0.1)
            fragColor.rgb += fog;
    }

    fragColor.rgb *= vec3(0.9,0.9,1.0);
}
