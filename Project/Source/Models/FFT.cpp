#include "FFT.h"

FFT::FFT(size_t nSamples, unsigned int planFlags) {
	this->in = (double*)fftw_malloc(sizeof(double) * nSamples);
	this->out = (double*)fftw_malloc(sizeof(double) * nSamples);
	this->plan = fftw_plan_r2r_1d(nSamples, this->in, this->out, FFTW_R2HC, planFlags);
};

FFT::~FFT() {
	fftw_destroy_plan(this->plan);
	fftw_free(in); fftw_free(out);
};

void FFT::Execute() {
	fftw_execute(this->plan);
}