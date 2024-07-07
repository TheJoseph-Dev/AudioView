#VERTEX_SHADER
#version 460 core

layout(location = 0) in vec3 position;
//layout(location = 1) in vec2 texCoords;
//layout(location = 2) in vec3 normals;

uniform float iTime;
uniform mat4 MVP;
uniform mat4 model;

//out vec2 TexCoords;
out vec3 Normals;
out vec3 lightPos; // World Space Position
out vec3 FragPos; // World Space Position

uniform int nSamples;
layout (std430, binding=10) readonly buffer fftData {
   float fft[];
};

void main() {
	//TexCoords = vec2(texCoords.x, 1.0-texCoords.y);
	Normals = transpose(inverse(mat3(model))) * normalize(position);
	
	lightPos = vec3(1.0);

	vec3 pos = position;
	int sampleIndex = int((pos.y+1.0)/2.0 * nSamples);
	pos += (pos * smoothstep( fft[sampleIndex]*0.01, 0.5, 0.55))*0.5;
	//pos += pos*((sin(iTime+gl_VertexID)+1.0)/2.0)*0.2;

	FragPos = vec3( model * vec4(pos, 1.0) );

	gl_Position = MVP*vec4(pos, 1.0);
}

#FRAGMENT_SHADER
#version 460 core

layout(location = 0) out vec4 fragColor;

//in vec2 TexCoords;
in vec3 Normals;
in vec3 FragPos;
in vec3 lightPos;

uniform float iTime;
//uniform int isEmissive;

#define PI 3.141592653

void main() {
	
	vec3 lightDir = normalize( lightPos - FragPos );
	const float ambientL = 0.1;
	float objLight = dot(normalize(Normals), lightDir)+1.0;
	objLight/=2.0;
	vec3 color = vec3(1.0);

	fragColor = vec4( objLight * color, 1.0 );

}