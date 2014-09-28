#ifndef DUMMYEFFECT_H
#define DUMMYEFFECT_H

#include "FxBase.h"
#include "Flanger.h"


using namespace FLANGER;

class DummyEffect : public FxBase
{
public:
	DummyEffect();
	virtual ~DummyEffect();

	void setSampleRate(float sampleRate);
	void setParameter(int index, float value);    //float value 0..1
	
	void resetBuffer();
    void resetCoeffs();
	void calc();

    void process(float *in, int sampleFrames);
    void process(float *inL, float *inR, int sampleFrames);
    void process(double *in, int sampleFrames);
    void process(double *inL, double *inR, int sampleFrames);

private:
	Flanger flanger_;
	
};

#endif