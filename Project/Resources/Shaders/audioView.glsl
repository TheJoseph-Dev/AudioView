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
uniform float iTime;
uniform float maxFreq;
uniform sampler1D audioFreqsTex;
uniform sampler2D bgImage;

#define PI 3.141592653

#define MAX_DEPTH 60
#define MAX_DIST 100.0

struct Ray {
    vec3 origin;
    vec3 dir;
};

struct SceneObject {
    vec3 pos;
    vec3 normals;
    vec3 color;
};

struct Scene {
    float d;
    SceneObject hitObj;
};

vec4 hueShift(in vec3 Color, in float Shift)
{
    vec3 P = vec3(0.55735)*dot(vec3(0.55735),Color);
    
    vec3 U = Color-P;
    
    vec3 V = cross(vec3(0.55735),U);    

    Color = U*cos(Shift*6.2832) + V*sin(Shift*6.2832) + P;
    
    return vec4(Color,1.0);
}

float fftx = 0.0;
float sphere(vec3 p, float r) { return length(p)-r - fftx; }

vec3 sphNormals(Ray ray, float d) {
    return ray.origin + (ray.dir*d);
}

float fresnel(float n1, float n2, vec3 normal, vec3 incident, float f0, float f90)
{
        // Schlick aproximation
        float r0 = (n1-n2) / (n1+n2);
        r0 *= r0;
        float cosX = -dot(normal, incident);
        if (n1 > n2)
        {
            float n = n1/n2;
            float sinT2 = n*n*(1.0-cosX*cosX);
            // Total internal reflection
            if (sinT2 > 1.0)
                return f90;
            cosX = sqrt(1.0-sinT2);
        }
        float x = 1.0-cosX;
        float ret = r0+(1.0-r0)*x*x*x*x*x;
 
        // adjust reflect multiplier for object reflectivity
        return mix(f0, f90, ret);
}


Scene map(vec3 ro) {
    
    float sDistance = MAX_DIST;
    SceneObject sObj;
    
    {
        vec3 s1Pos = vec3(0.0, 0.0, -0.0);
        float s1 = sphere(ro-s1Pos, 0.5);
        sDistance = min(sDistance, s1);

        if(sDistance == s1) sObj = SceneObject(s1Pos, vec3(0.0), vec3(0.2, 0.4, 0.8));
    }

    {
        vec3 sPos = vec3(cos(iTime)*2.0, cos(iTime)*0.75, sin(iTime))*0.5;
        float sph = sphere(ro-sPos, 0.05);
        //sDistance = min(sDistance, sph);

        //if(sDistance == sph) sObj = SceneObject(sPos, vec3(0.0), vec3(0.4, 0.6, 0.8));
    }

    return Scene(sDistance, sObj);
}

vec3 GetSurfaceNormal(vec3 p)
{
    float d0 = map(p).d;
    const vec2 epsilon = vec2(.0001,0);
    vec3 d1 = vec3(
        map(p-epsilon.xyy).d,
        map(p-epsilon.yxy).d,
        map(p-epsilon.yyx).d);
    return normalize(d0 - d1);
}

void raymarcher(in Ray ray, out Scene scene) {

    for(int i = 0; i < MAX_DEPTH; i++) {
        scene = map(ray.origin);
        ray.origin += ray.dir*scene.d;
        scene.hitObj.normals = scene.d < 0.1 ? GetSurfaceNormal(ray.origin) : vec3(0.0); // Miss
    }
}

mat2 rotation(float angle) { return mat2(cos(angle), -sin(angle), sin(angle), cos(angle)); }

void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = gl_FragCoord.xy/iResolution.xy * 2.0 - 1.0;
    float aspect = iResolution.x / iResolution.y;
    uv.x *= aspect;
    
    float fft = texture( audioFreqsTex, pow(uv.y/5.0 + 0.5, 5) ).x;
    fftx = smoothstep(0.0, 4.0, fft); 
    
    const float fov = 1.0;
    Ray ray = Ray( vec3(0.0, 0.0, -1.0), normalize(vec3(uv*fov, 1.0)) );
    //ray.dir.xz *= rotation(sin(iTime*0.5)*0.5);
    //ray.dir = normalize(ray.dir);

    Scene scene;
    const float zDepth = 1.0;
    raymarcher(ray, scene);

    vec2 bgUV = gl_FragCoord.xy/iResolution.xy;
    bgUV.y = 1.0-bgUV.y;
    vec4 bgColor = texture(bgImage, bgUV); //vec4(0.5, 0.7, 0.9, 1.0);
    if(scene.d >= MAX_DIST) { fragColor = bgColor; return; } // Miss

    SceneObject hitObj = scene.hitObj;
    vec3 lightPos = vec3(0.0, 1.0, 0.0); //vec3(cos(iTime), 1.0, sin(iTime));
    float diffuse = dot((hitObj.normals+1.0)/2.0, normalize(lightPos));
    vec4 hue = hueShift(vec3(1.0, 0.0, 0.0), maxFreq);
    vec3 dfColor =  hue.rgb * mix(diffuse, fft, 0.75);
    float fresnelEffect = fresnel(1.0, 0.98, ray.dir, hitObj.normals, 0.9, 1.0);

    vec3 color = dfColor + smoothstep(0.85, 1.0, fresnelEffect)*hue.rgb;
    fragColor = vec4(mix(bgColor.rgb, color, smoothstep(0.82, 1.0, fresnelEffect)), 1.0);
}