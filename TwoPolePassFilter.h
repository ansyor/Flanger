#ifndef TWOPOLELOWPASSFILTER_H
#define TWOPOLELOWPASSFILTER_H

class TwoPoleLowPassFilter
{
public:
	TwoPoleLowPassFilter( float cutoff);
	~TwoPoleLowPassFilter();

	void setSampleRate(float sampleRate);

	double process(double f);

private:
    double x1, x2, y1, y2;
    double a0, a1, a2, b0, b1, b2, w0, alpha;
	float sampleRate_;

};

#endif