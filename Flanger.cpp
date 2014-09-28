#include "Flanger.h"
#include <string.h>
#include <math.h>

using namespace FLANGER;

Flanger::Flanger()
{
	// costructor
	paramSweepRate = 0.1f;
	paramWidth = 0.3f;
	paramFeedback = 0.0f;
	paramDelay = 0.2f;
	paramMix = 1.0f;

	// allocate the buffers
	_bufferL = new double[MAXBUFFERSIZE];
	_bufferR = new double[MAXBUFFERSIZE];
	
	for( int i = 0; i < MAXBUFFERSIZE; i++)
	{
		_bufferL[i] = _bufferR[i] = 0.0;
	}

	_delayControlFilter = new TwoPoleLowPassFilter( 20.0f );
	_mixControlFilter = new TwoPoleLowPassFilter( 20.0f );
}

Flanger::~Flanger()
{
	delete _delayControlFilter;
	delete _mixControlFilter;

	if( _bufferL )
		delete[] _bufferL;
	if( _bufferR )
		delete[] _bufferR;
}

void Flanger::resetBuffer()
{
	_outL = 0;
	_outR = 0;
}

void Flanger::resetCoeffs()
{
}

void Flanger::setParameter(int index, float value)
{
	{
	switch(index)
	{
	case kRate :
		parameter_[kRate] = value;
		paramSweepRate = parameter_[kRate];
		break;
	
	case kWidth :
		parameter_[kWidth] = value;
		paramWidth = parameter_[kWidth];
		break;

	case kFeedback :
		parameter_[kFeedback] = value;
		_feedback = parameter_[kFeedback];
		break;

	case kDelay :
		parameter_[kDelay] = value;
		paramDelay = parameter_[kDelay];
		break;

	case kMix :
		parameter_[kMix] = value;
		paramMix = parameter_[kMix];
		break;

	case kWetLevel :
		parameter_[kWetLevel] = value;
		wetLevel = parameter_[kWetLevel];
		break;
	}
}
}

void Flanger::setSampleRate(float sampleRate)
{
	sampleRate_ = sampleRate;
	_delayControlFilter->setSampleRate(sampleRate_);
	_mixControlFilter->setSampleRate(sampleRate_);
}

void Flanger::setRate()
{
	_sweepRate = pow(10.0,(double)paramSweepRate);
	_sweepRate  -= 1.0;
	_sweepRate  *= 1.05556f;
	_sweepRate  += 0.05f;

	// finish setup
	setSweep();
}

void Flanger::setWidth()
{
	// map so that we can spec between 0ms and 10ms
	if( paramWidth <= 0.05)
	{
		_sweepSamples = 0.0;
	}
	else
	{
		_sweepSamples = paramSweepRate * 0.01 * sampleRate_;
	}

	// finish setup
	setSweep();
}

void Flanger::setSweep()
{
	_step = (double)(_sweepSamples * 2.0 * _sweepRate) / sampleRate_;

	// calc max and start sweep at 0.0
	_sweep = 0.0;
	_maxSweepSamples = _sweepSamples;
}

void Flanger::process(float &in)
{
		double inmixL = in + _feedback * _outL;

		_bufferL[writeIndex] = inmixL;
		
		 writeIndex = (writeIndex + 1) & (MAXBUFFERSIZE-1);

		// get filtered delay
		double delay = _delayControlFilter->process(paramDelay);
		
		// delay 0.0-1.0 maps to 0.02ms to 10ms (always have at least 1 sample of delay)
		double delaySamples = (delay * sampleRate_ * 0.01) + 1.0;
		delaySamples += _sweep;

		int ep1, ep2;
		double w1, w2;
		double ep = writeIndex - delaySamples;
		
		if( ep < 0.0)
		{
			ep += MAXBUFFERSIZE;
		}
		
		MODF(ep, ep1, w2);
		ep1 &= (MAXBUFFERSIZE-1);
		ep2 = ep1 + 1;
		ep2 &= (MAXBUFFERSIZE-1);
		w1 = 1.0 - w2;
		_outL = _bufferL[ep1] * w1 + _bufferL[ep2] * w2;

		// get filtered mix
		double mix = _mixControlFilter->process(paramMix);

		// develop output mix
		 in =  (1.f- wetLevel) * in + wetLevel * (float)mix * (float)_outL; 

		// see if we're doing sweep
		if( _step != 0.0)
		{
			// increment the sweep
			_sweep += _step;
			if( _sweep <= 0.0)
			{
				_sweep = 0.0;
				// and reverse direction
				_step = -_step;
			}
			else if( _sweep >=  _maxSweepSamples)
			{
				_step = -_step;
			}
		}
	}


void Flanger::process(float &inL, float &inR)
{
		double inmixL = inL + _feedback * _outL;
		double inmixR = inR + _feedback *  _outR;

		_bufferL[writeIndex] = inmixL;
		_bufferR[writeIndex] = inmixR;
		
		 writeIndex = (writeIndex + 1) & (MAXBUFFERSIZE-1);

		// get filtered delay
		double delay = _delayControlFilter->process(paramDelay);
		
		// delay 0.0-1.0 maps to 0.02ms to 10ms (always have at least 1 sample of delay)
		double delaySamples = (delay * sampleRate_ * 0.01) + 1.0;
		delaySamples += _sweep;

		int ep1, ep2;
		double w1, w2;
		double ep = writeIndex - delaySamples;
		
		if( ep < 0.0)
		{
			ep += MAXBUFFERSIZE;
		}
		
		MODF(ep, ep1, w2);
		ep1 &= (MAXBUFFERSIZE-1);
		ep2 = ep1 + 1;
		ep2 &= (MAXBUFFERSIZE-1);
		w1 = 1.0 - w2;
		_outL = _bufferL[ep1] * w1 + _bufferL[ep2] * w2;
		_outR = _bufferR[ep1] * w1 + _bufferR[ep2] * w2;

		// get filtered mix
		double mix = _mixControlFilter->process(paramMix);

		// develop output mix
		inL =  (1- wetLevel) * inL + wetLevel *(float) mix * (float)_outL; 
		inR =  (1- wetLevel) * inR + wetLevel *(float) mix * (float)_outR;

		// see if we're doing sweep
		if( _step != 0.0)
		{
			// increment the sweep
			_sweep += _step;
			if( _sweep <= 0.0)
			{
				// make sure we don't go negative
				_sweep = 0.0;
				// and reverse direction
				_step = -_step;
			}
			else if( _sweep >=  _maxSweepSamples)
			{
				_step = -_step;
			}
		}
	}


void Flanger::process(double &in)
{
		double inmixL = in + _feedback * _outL;

		_bufferL[writeIndex] = inmixL;
		
		 writeIndex = (writeIndex + 1) & (MAXBUFFERSIZE-1);

		// get filtered delay
		double delay = _delayControlFilter->process(paramDelay);
		
		// delay 0.0-1.0 maps to 0.02ms to 10ms (always have at least 1 sample of delay)
		double delaySamples = (delay * sampleRate_ * 0.01) + 1.0;
		delaySamples += _sweep;

		int ep1, ep2;
		double w1, w2;
		double ep = writeIndex - delaySamples;
		
		if( ep < 0.0)
		{
			ep += MAXBUFFERSIZE;
		}
		
		MODF(ep, ep1, w2);
		ep1 &= (MAXBUFFERSIZE-1);
		ep2 = ep1 + 1;
		ep2 &= (MAXBUFFERSIZE-1);
		w1 = 1.0 - w2;
		_outL = _bufferL[ep1] * w1 + _bufferL[ep2] * w2;

		// get filtered mix
		double mix = _mixControlFilter->process(paramMix);

		// develop output mix
		 in =  (1- wetLevel) * in + wetLevel * mix * _outL; 

		// see if we're doing sweep
		if( _step != 0.0)
		{
			// increment the sweep
			_sweep += _step;
			if( _sweep <= 0.0)
			{
				// make sure we don't go negative
				_sweep = 0.0;
				// and reverse direction
				_step = -_step;
			}
			else if( _sweep >=  _maxSweepSamples)
			{
				_step = -_step;
			}
		}
	}


void Flanger::process(double &inL, double &inR)
{
		double inmixL = inL + _feedback  * _outL;
		double inmixR = inR + _feedback  * _outR;

		_bufferL[writeIndex] = inmixL;
		_bufferR[writeIndex] = inmixR;
		
		 writeIndex = (writeIndex + 1) & (MAXBUFFERSIZE-1);

		// get filtered delay
		double delay = _delayControlFilter->process(paramDelay);
		
		// delay 0.0-1.0 maps to 0.02ms to 10ms (always have at least 1 sample of delay)
		double delaySamples = (delay * sampleRate_ * 0.01) + 1.0;
		delaySamples += _sweep;

		int ep1, ep2;
		double w1, w2;
		double ep = writeIndex - delaySamples;
		
		if( ep < 0.0)
		{
			ep += MAXBUFFERSIZE;
		}
		
		MODF(ep, ep1, w2);
		ep1 &= (MAXBUFFERSIZE-1);
		ep2 = ep1 + 1;
		ep2 &= (MAXBUFFERSIZE-1);
		w1 = 1.0 - w2;
		_outL = _bufferL[ep1] * w1 + _bufferL[ep2] * w2;
		_outR = _bufferR[ep1] * w1 + _bufferR[ep2] * w2;

		// get filtered mix
		double mix = _mixControlFilter->process(paramMix);

		// develop output mix
		inL =  (1- wetLevel) * inL + wetLevel * mix * _outL; 
		inR =  (1- wetLevel) * inR + wetLevel * mix * _outR;

		// see if we're doing sweep
		if( _step != 0.0)
		{
			// increment the sweep
			_sweep += _step;
			if( _sweep <= 0.0)
			{
				// make sure we don't go negative
				_sweep = 0.0;
				// and reverse direction
				_step = -_step;
			}
			else if( _sweep >=  _maxSweepSamples)
			{
				_step = -_step;
			}
		}
	}
