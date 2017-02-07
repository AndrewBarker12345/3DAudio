/*
 Resampler.h
 
 Resamples an audio stream.

 Copyright (C) 2017  Andrew Barker
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 The author can be contacted via email at andrew.barker.12345@gmail.com.
*/

#ifndef __Resampler__
#define __Resampler__

// NOTE: this whole setup assumes that 1/fs_out !> N_in / fs_in. that is that there is not less than one sample per output buffer on average.
class Resampler
{
public:
    Resampler() noexcept {};
    Resampler(double new_fs_in, int new_N_in, double new_fs_out, bool direction) noexcept;
    ~Resampler() {};
    void resampleLinear(const float* x, float* y) noexcept;
    void unsampleLinear(const float* x, int Nin, float* y) noexcept;
    int getNout() const noexcept;
    int getNoutMax() const noexcept ;
    //int getNumSamplesLatency();
private:
    // input sample rate
    double fs_in = 44100;
    // output sample rate
    double fs_out = 44100;
    // length of input buffer
    int N_in = 0;
    // length of output buffer (fraction due to sample rate change, however resample only outputs floor(N_out) samples per input buffer and the appropriate time shifting is taken care of with the computation of those samples)
    double N_out = 0;
    // direction of resampling: true = resample, false = unsample
    bool dir = true;
    // last sample of the previous input
    double prevSample = 0;
    // the time offset of the first resampled sample from the actual begin time of the input buffer, due to the fact that N_out is fractional in nature
    double offset = 0;
    // state variable(s) to indicate when we are outputing the occasional "short" buffer from the resampler and when we are indexing into "short" buffers when unsampling
    bool shortBuffer = false;
};

#endif /* defined(__Resampler__) */
