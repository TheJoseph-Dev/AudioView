#include <iostream>
#include "Models/FFT.h"
#include <SFML/Audio.hpp>
#include "AudioFile.h"
#include <cmath>
#include <conio.h>

#define Resources(path) "Project/Resources/" path

#define KEY_VOLUME_P '+'
#define KEY_VOLUME_M '-'

int main() {
	
	sf::Music music;
	AudioFile<double> af;
	std::string song = "In-My-Life-MA.wav";
	
	af.load(Resources("Samples/" + song));
	af.printSummary();

	if (!music.openFromFile(Resources("Samples/" + song)))
		return -1;

	music.play();

	size_t nSamples = af.getSampleRate() / 4;
	FFT musicFFT = FFT(nSamples, FFTW_MEASURE);

	system(" ");

	
	while(music.getStatus() == music.Playing) {

		if (_kbhit()) {
			char c = _getch();
			if (c == KEY_VOLUME_P) music.setVolume(music.getVolume() + 10);
			if (c == KEY_VOLUME_M) music.setVolume(music.getVolume() - 10);
		}

		printf("\x1b[2j\x1b[H");
		float seconds = music.getPlayingOffset().asSeconds();
		double currentSample = af.samples[0][af.getSampleRate() * seconds];
		puts("\n\n\n\n\n\n\n\n");
		printf("\tNow Playing: %s\n\tTime: %.2f s\n\tVolume: %.2f\n\tSample Rate: %u", song.c_str(), seconds, music.getVolume(), music.getSampleRate());
		

		double maxVolumeL = 0, maxVolumeR = 0;
		for (int i = 0; i < nSamples; i++) {
			seconds = music.getPlayingOffset().asSeconds();
			if (seconds >= music.getDuration().asSeconds() - 1) break;
			int sampleIndex = af.getSampleRate() * seconds + i;
			maxVolumeL = std::max(maxVolumeL, abs(af.samples[0][sampleIndex]) * music.getVolume() / 100.0);
			maxVolumeR = std::max(maxVolumeR, abs(af.samples[1][sampleIndex]) * music.getVolume()/100.0);
			musicFFT.in[i] = af.samples[0][sampleIndex];
		}

		musicFFT.Execute();
		
	}

}