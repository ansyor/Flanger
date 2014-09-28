#include "TwoPolePassFilter.h"
#include "math.h"

#define myPI	3.1415926535897932384626433832795

TwoPoleLowPassFilter::TwoPoleLowPassFilter( float cutoff)
{
	double Q = 1.1f;
    w0 = 2 * myPI * cutoff / sampleRate_;
    alpha = sin(w0) / (2.0 * Q);
    b0 =  (1.0 - cos(w0))/2;
    b1 =   1.0 - cos(w0);
    b2 =  (1.0 - cos(w0))/2;
    a0 =   1.0 + alpha;
    a1 =  -2.0 * cos(w0);
    a2 = 1.0 - alpha;
	x1 = x2 = y1 = y2 = 0;

}

TwoPoleLowPassFilter::~TwoPoleLowPassFilter()
{
}

void TwoPoleLowPassFilter::setSampleRate(float sampleRate)
{
	sampleRate_ = sampleRate;
}

double TwoPoleLowPassFilter::process(double x)
{
    double y = (b0/a0)*x + (b1/a0)*x1 + (b2/a0)*x2 - (a1/a0)*y1 - (a2/a0)*y2;                  
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = y;
    return y;
}
