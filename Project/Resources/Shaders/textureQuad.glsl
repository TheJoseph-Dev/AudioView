#VERTEX_SHADER
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;

uniform mat4 MVP;

out vec2 TexCoords;

void main() {
	TexCoords = vec2(texCoords.x, 1.0-texCoords.y);

	gl_Position = MVP*vec4(position, 1.0);
}

#FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 fragColor;

in vec2 TexCoords;

uniform vec2 iResolution;
uniform sampler1D tex;

#define PI 3.141592653

#define MAX_DEPTH 60
#define MAX_DIST 100.0

struct Ray {
    vec3 origin;
    vec3 dir;
};

float fftx = 0.0;
float sphere(vec3 p, float r) { return length(p)-r - fftx; }

float map(vec3 ro) {
    
    float sDistance = MAX_DIST;
    
    vec3 s1Pos = ro-vec3(0.0, 0.0, -0.0);
    float s1 = sphere(s1Pos, 0.5);
    
    sDistance = min(sDistance, s1);
    
    return sDistance;
}

void raymarcher(in Ray ray, out float d) {

    for(int i = 0; i < MAX_DEPTH; i++) {
        //ray.origin += ;
        d += map(ray.origin+ray.dir*d);
    }
}


void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = gl_FragCoord.xy/iResolution.xy * 2.0 - 1.0;
    float aspect = iResolution.x / iResolution.y;
    uv.x *= aspect;
    
    float fft = texture( tex, pow(uv.y/5.0 + 0.5, 5)).x;
    fftx = smoothstep(0, 4.0, fft); 
    
    Ray ray = Ray( vec3(0.0, 0.0, -1.0), normalize(vec3(uv, 1.0)) );
    
    float d = 0.0;
    const float zDepth = 1.0;
    raymarcher(ray, d);
    fragColor = vec4(d);
}