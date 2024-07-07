#include <glad/glad.h>
#include <glfw3.h>
#include <SFML/Audio.hpp>
#include <cmath>

#include "AudioFile/AudioFile.h"
#include "Models/FFT.h"

#include "Utils.h"
#include "Models/OpenGL/Framebuffer.h"
#include "Models/OpenGL/OBJLoader.h"
#include "Models/OpenGL/Shader.h"
#include "Models/OpenGL/Camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

#define WINDOW_WIDTH 960.0f
#define WINDOW_HEIGHT 540.0f


#define KEY_VOLUME_P '+'
#define KEY_VOLUME_M '-'

float lerp(float v0, float v1, float t) {
	return (1 - t) * v0 + t * v1;
}

int main() {
	
	// SETUP ---
	// Init GLFW
	if (!glfwInit()) { print("ERROR: Could not Init GLFW");  return -1; }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "AudioView", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { print("ERROR: Could not Init GLAD"); return -1; }


	// Tell OpenGL the size of the rendering window so OpenGL knows how we want to display the data and coordinates with respect to the window.
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Window Resize Callback
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 3D:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST); // Depth Testing
	glDisable(GL_CULL_FACE);

	// * End Init ---

	sf::Music music;
	AudioFile<double> af;
	std::string song = "In-My-Life.wav";//"In-My-Life-MA.wav";
	
	af.load(Resources("Samples/" + song));
	af.printSummary();

	if (!music.openFromFile(Resources("Samples/" + song)))
		return -1;

	music.play();

	size_t nSamples = 2048; //af.getSampleRate() / 32;
	FFT musicFFT = FFT(nSamples, FFTW_MEASURE);
	float* lastFFT = new float[nSamples]();

	Camera camera = Camera(glm::vec3(0.0f, 0.0f, -1.0f));

	FrameBuffer fb = FrameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
	fb.Check();
	fb.Unbind();

	IOBJLoader obj(Resources("Objects/sphereLowRes.obj"));
	float* vertices = obj.GetVertices();
	int verticesSize = obj.GetVerticesSize();
	print("Vetices: " << obj.GetVerticesSize() / 3 << "\t" << "Floats: " << obj.GetVerticesSize());

	unsigned int VBO;
	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(float), vertices, GL_STATIC_DRAW);

	unsigned int IBO;
	glCreateBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.GetIndicies().size() * sizeof(int), obj.GetIndicies().data(), GL_STATIC_DRAW);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, Vertex::GetStride() * sizeof(float), (void*)(sizeof(float) * 3));

	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, Vertex::GetStride() * sizeof(float), (void*)(sizeof(float) * 5));

	constexpr unsigned short stride = 5;
	float quadVertices[] = {
		0.0, 0.0, 0.0,	 0.0, 0.0,
		1.0, 0.0, 0.0,	 1.0, 0.0,
		0.0, 1.0, 0.0,	 0.0, 1.0,

		1.0, 0.0, 0.0,	 1.0, 0.0,
		0.0, 1.0, 0.0,	 0.0, 1.0,
		1.0, 1.0, 0.0,	 1.0, 1.0
	};

	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	unsigned int quadVAO;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 3));


	Shader shader = Shader(Resources("Shaders/textureQuad.glsl"));
	shader.Bind();

	glm::mat4 mvp = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.f);
	glm::mat4 model = glm::mat4(1.0f);
	//glm::scale(model, glm::vec3(0.25f));
	model = glm::scale(model, glm::vec3(WINDOW_WIDTH, WINDOW_HEIGHT, 1));

	glm::mat4 planeProjection = glm::ortho(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);
	fb.SetTransform(planeProjection);
	
	mvp *= planeProjection;
	//mvp *= camera.GetView();
	mvp *= model;

	shader.SetUniformMat4("MVP", mvp);
	//shader.SetUniformMat4("model", model);

	unsigned int fftTextureID;
	glGenTextures(1, &fftTextureID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, fftTextureID);

	// Texture Wrap/Filtering options and Mipmap
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	system(" ");
	
	float time = 0;
	while(!glfwWindowShouldClose(window)) {

		/*if (_kbhit()) {
			char c = _getch();
			if (c == KEY_VOLUME_P) music.setVolume(music.getVolume() + 10);
			if (c == KEY_VOLUME_M) music.setVolume(music.getVolume() - 10);
		}*/

		float seconds = music.getPlayingOffset().asSeconds();
		/*
		printf("\x1b[2j\x1b[H");
		double currentSample = af.samples[0][af.getSampleRate() * seconds];
		puts("\n\n\n\n\n\n\n\n");
		printf("\tNow Playing: %s\n\tTime: %.2f s\n\tVolume: %.2f\n\tSample Rate: %u", song.c_str(), seconds, music.getVolume(), music.getSampleRate());
		*/

		double maxVolumeL = 0, maxVolumeR = 0;
		for (int i = 0; i < nSamples; i++) {
			seconds = music.getPlayingOffset().asSeconds();
			if (seconds >= music.getDuration().asSeconds() - 1) break;
			int sampleIndex = af.getSampleRate() * seconds + i;
			maxVolumeL = std::max(maxVolumeL, abs(af.samples[0][sampleIndex]) * music.getVolume() / 100.0);
			maxVolumeR = std::max(maxVolumeR, abs(af.samples[1][sampleIndex]) * music.getVolume() / 100.0);
			musicFFT.in[i] = (af.samples[0][sampleIndex] + af.samples[1][sampleIndex])/2.0; // Stereo to Mono
		}

		//for (int i = 0; i < nSamples; i++) lastFFT[i] = musicFFT.getResultF()[i];
		musicFFT.Execute( FFT_BM_WINDOW | FFT_ABS | FFT_SMOOTH | FFT_CONVERT_TO_DB  );
		for (int i = 0; i < nSamples; i++) lastFFT[i] = lerp(lastFFT[i], musicFFT.getResultF()[i], 0.1f);
		
		fb.Bind();
		//glEnable(GL_DEPTH_TEST);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.7f, 0.8f, 1.0f);
		time = glfwGetTime();
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, fftTextureID);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, nSamples, 0, GL_RED, GL_FLOAT, lastFFT );
		
		shader.Bind();
		//shader.SetUniformFloat("iTime", time);
		//shader.SetUniformInt("nSamples", nSamples);
		
		shader.SetUniformInt("tex", 0);
		shader.SetUniform2f("iResolution", WINDOW_WIDTH, WINDOW_HEIGHT);

		// Index buffer
		/*
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glDrawElements( GL_TRIANGLES, obj.GetIndicies().size(),  GL_UNSIGNED_INT, (void*)0 );
		*/
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		fb.Unbind();
		fb.Draw();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}