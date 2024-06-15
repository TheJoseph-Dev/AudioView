#ifndef FFT_H
#define FFT_H

#include <fftw3.h>

#define LOW_FREQUENCY 20
#define HIGH_FREQUENCY 20000

class FFT {

	double* out;
	fftw_plan plan;

public:
	double* in;

public:
	FFT(size_t nSamples, unsigned int planFlags = FFTW_ESTIMATE);
	~FFT();

	void Execute();

	inline double* getResult() const { return this->out; };
};

#endif // !FFT_H
