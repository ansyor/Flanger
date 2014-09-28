#ifndef FLANGER_H
#define FLANGER_H

#include "math.h"
#include "FxBase.h"
#include "ModuleBase.h"
#include "TwoPolePassFilter.h"

#define	MAXBUFFERSIZE 8192		// must be about 1/5 of a second at given sample rate and must be a power of 2 for masking
#define ROUND(n)		((int)((double)(n)+0.5))
#define PIN(n,min,max) ((n) > (max) ? max : ((n) < (min) ? (min) : (n)))

#define	MODF(n,i,f) ((i) = (int)(n), (f) = (n) - (double)(i))
//	MODF - vaguely related to the library routine modf(), this macro breaks a double into
//	integer and fractional components i and f respectively.
//
//	n - input number, a double
//	i - integer portion, an integer (the input number integer portion should fit)
//	f - fractional portion, a double

namespace FLANGER
{
enum t
{
	// Parameters Tags
	kRate = 0,
	kWidth,
	kFeedback,
	kDelay,
	kMix,
	kWetLevel,

	kNumParams,
};
};

class Flanger : public ModuleBase
{
public:
	Flanger();
	~Flanger();
	
	void setSampleRate(float sampleRate);
	void setParameter(int index, float value);
	
	void resetBuffer();
	void resetCoeffs();
	
	void setRate();
	void setWidth();
	void setSweep();
	
	void process(float &in);
	void process(float &inL, float &inR);
	void process(double &in);
	void process(double &inL, double &inR);
	
private:	
	float *parameter_;

	float paramSweepRate;
	float paramWidth;
	float paramFeedback;
	float paramDelay;
	float paramMix;
	float wetLevel;

	double _sweepRate;			// actual calc'd sweep rate
	double _feedback;			// 0.0 to 1.0;
	double _sweepSamples;			// sweep width in # of samples
	double _maxSweepSamples;	// upper bound of sweep in samples
	
	double *_bufferL;				// stored sound
	double *_bufferR;				// stored sound
	int	   writeIndex;					// fill/write pointer
	double _step;				// amount to step the sweep each sample
	double _sweep;				// current value of sweep in steps behind fill pointer
	double _outL;			// most recent output value (for feedback)
	double _outR;			// most recent output value (for feedback)

	TwoPoleLowPassFilter *_delayControlFilter;
	TwoPoleLowPassFilter *_mixControlFilter;
};

#endif
