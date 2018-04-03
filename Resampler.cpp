//
//  Resampler.cpp
//
//  Created by Andrew Barker on 9/30/14.
//
//
/*
     3DAudio: simulates surround sound audio for headphones
     Copyright (C) 2016  Andrew Barker
     
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

#include "Resampler.h"

Resampler::Resampler(double new_fs_in, int new_N_in_out, double new_fs_out, bool direction) noexcept
    : fs_in(new_fs_in), fs_out(new_fs_out), dir(direction)
{
    if (dir) {
        // 2nd param is N_in for the resampler
        N_in = new_N_in_out;
        N_out = fs_out * ((double)N_in) / fs_in;
    } else {
        // 2nd param is N_out for the unsampler
        N_out = new_N_in_out;
        N_in = fs_in * N_out / fs_out;
        N_in += 1;
    }
}

// linearly resamples N_in samples in x at fs_in and spits out ceil(N_out) samples in y at fs_out
void Resampler::resampleLinear(const float* x, float *y) noexcept
{
    int Nout = N_out;//ceil(N_out);
    const double this_fs_in = fs_in;
    const double this_fs_out = fs_out;
    const double Ts_in = 1.0 / this_fs_in;
    const double Ts_out = 1.0 / this_fs_out;
    const double this_offset = offset;
    const double thisPrevSample = prevSample;
    if (((double)Nout) * Ts_out + this_offset >= ((double)N_in) * Ts_in) {
        y[Nout] = 0; // set last output sample to 0 since it is not technically in this output buffer
        shortBuffer = true;
    } else {
        Nout += 1;
        shortBuffer = false;
    }
    const int thisNout = Nout;
    int k_low;
    float x_low, x_high;
    double t_out, t_in;
    for (int n = 0; n < thisNout; ++n) {
        t_out = ((double)n) * Ts_out + this_offset;
        k_low = t_out * this_fs_in;
        if (k_low == 0) {
            x_low = thisPrevSample;
        } else {
            x_low = x[k_low-1];
        }
        x_high = x[k_low];
        t_in = ((double)k_low) * Ts_in;
        // y = b     + m                               * x
        y[n] = x_low + ((x_high - x_low) * this_fs_in) * (t_out - t_in);
    }
    offset = (((double)thisNout) * Ts_out + this_offset) - ((double)N_in) * Ts_in;
    prevSample = x[N_in-1];
}

void Resampler::unsampleLinear(const float* x, const int Nin, float* y) noexcept
{
    const double this_fs_in = fs_in;
    const double this_fs_out = fs_out;
    const double Ts_in = 1.0 / this_fs_in;
    const double Ts_out = 1.0 / this_fs_out;
    const double this_offset = offset;
    const double thisPrevSample = prevSample;
    const int thisNout = N_out;
    int k_low;
    float x_low, x_high;
    double t_out, t_in;
    for (int n = 0; n < thisNout; ++n) {
        t_out = ((double)n) * Ts_out + this_offset;
        k_low = t_out * this_fs_in;
        if (k_low == 0) {
            x_low = thisPrevSample;
        } else {
            x_low = x[k_low-1];
        }
        x_high = x[k_low];
        t_in = ((double)k_low) * Ts_in;
        // y = b     + m                             * x
        y[n] = x_low + (x_high - x_low) * this_fs_in * (t_out - t_in);
    }
    offset = (((double)thisNout) * Ts_out + this_offset) - ((double)Nin) * Ts_in;
    prevSample = x[Nin-1];
}

int Resampler::getNout() const noexcept
{
    if (dir) {
        // resampling, resamplers have variable buffer size due to the fractional nature of fs_in/fs_out
        if (shortBuffer) {
            return N_out;
        } else {
            return N_out + 1;
        }
    } else {
        // unsampling, unsamplers should always output the same number of samples per buffer as was initially fed to the associated resampler
        return N_out;
    }
}

// returns the maximum size of the output buffer (used for allocating the output buffer's memory)
int Resampler::getNoutMax() const noexcept
{
    if (dir)
        return N_out + 1;
    else
        return N_out;
}

//int Resampler::getNumSamplesLatency()
//{
//    if (dir)
//        return 1;
//    else
//        return 0;
//}

//#include <math.h>
//// default construtor
//Resampler::Resampler()
//{
//}
//
//// the useful construtor
//Resampler::Resampler(double new_fs_in, int new_N_in_out, double new_fs_out, int numBuffersDelay, int direction)
//{
//    fs_in = new_fs_in;
//    fs_out = new_fs_out;
//    dir = direction;
//    if (dir == 0) {
//        // 2nd param is N_in for the resampler
//        N_in = new_N_in_out;
//        N_out = fs_out*((double)N_in)/fs_in;
//    } else {
//        // 2nd param is N_out for the unsampler
//        N_out = new_N_in_out;
//        N_in = ceil( fs_in*N_out/fs_out );
//    }
//    quality = numBuffersDelay;
//    
//    // allocate and zero init array of previous input buffers
//    inputs = new float* [2*quality+1];
//    for (int i = 0; i < 2*quality+1; i++) {
//        inputs[i] = new float[N_in];
//        for (int j = 0; j < N_in; j++) {
//            inputs[i][j] = 0;
//        }
//    }
//    
//    if (dir == 0) {
//        // resampler just needs to keep track if the one buffer length currently being output
//        shortBuffer[0] = false;
//    } else {
//        delete[] shortBuffer;
//        // unsampler needs to know if nearby buffers to the one currently being processed are short/normal length
//        shortBuffer = new bool[2*quality+1];
//        for (int i = 0; i < 2*quality+1; i++) {
//            shortBuffer[i] = false;
//        }
//    }
//    
//    inited = true;
//}
//
//// destructor
//Resampler::~Resampler()
//{
//}
//
//// linearly resamples N_in samples in x at fs_in and spits out ceil(N_out) samples in y at fs_out
//void Resampler::resampleLinear(float *x, float *y)
//{
//    if (inited) {
//        // shift the prevs inputs
//        for (int i = 2*quality; i >= 1; i--) {
//            // not sure if we can get away with this...
//            //inputs[i] = inputs[i-1];
//            for (int j = 0; j < N_in; j++) {
//                inputs[i][j] = inputs[i-1][j];
//            }
//        }
//        // ... and add the current input
//        for (int n = 0; n < N_in; n++) {
//            inputs[0][n] = x[n];
//        }
//        
//        // number of output samples for y
//        int Nout = ceil(N_out);
//        
//        if (numBuffersProcessed >= quality) {
//            // in this case we have passed the required processing latency so we can compute samples
//            
//            // last sample in this output buffer will be in the next input buffer's time region so this current output buffer needs to be one sample short to correct this, otherwise the time frame of our output buffers will eventually (perhaps very relatively soon) drift into future time that we don't have input samples for yet (due to the sample rate ratio being fractional in nature and hince the input/output buffer size ratio need be fractional so some output buffers are Nout-1 in length rather than Nout).
//            // NOTE: this should never be true for the unsampler (dir = 1), if so we have problems
//            // ALSO NOTE: the unsampler's frame will potentially not pass the negative version of this test and the offset (negative) magnitude will be more than one output sample period, but that is ok as Nin changes and is specified for this buffer by being passed in as a parameter
//            if (((double)(Nout-1))/fs_out + offset >= ((double)N_in)/fs_in) {
//                // set last output sample to 0 since it is not technically in this output buffer
//                y[Nout-1] = 0;
//                // don't compute the last output sample in the for loop below
//                Nout -= 1;
//                shortBuffer[0] = true;
//            } else {
//                shortBuffer[0] = false;
//            }
//            
//            // sample indecies of input buffer samples that boarder each output sample
//            int k_low, k_high;
//            // current time of each output/input sample, relative time for sinc function displacement
//            double t_out, t_in;
//            // localized slope for linear interpolation
//            double m;
//            // more variables, for indexing into inputs array
//            int i_low, i_high, j_low, j_high;
//            
//            for (int n = 0; n < Nout; n++) {
//                // compute shit
//                t_out = ((double)n)/fs_out + offset;
//                k_low = floor(t_out*fs_in);
//                k_high = k_low + 1;
//                t_in = ((double)k_low)/fs_in;
//                
//                // i indexes into which prev buffer we're needing, set it to the middle and adjust
//                i_low = quality;
//                i_high = quality;
//                // j is the indexing variable for the samples, so put it in range [0, N_in-1]
//                j_low = k_low;
//                j_high = k_high;
//                // adjust to proper indexing
//                while (j_low < 0) {
//                    j_low += N_in;
//                    i_low ++;
//                }
//                while (j_low >= N_in) {
//                    j_low -= N_in;
//                    i_low --;
//                }
//                while (j_high < 0) {
//                    j_high += N_in;
//                    i_high ++;
//                }
//                while (j_high >= N_in) {
//                    j_high -= N_in;
//                    i_high --;
//                }
//                
//                // y = mx + b
//                m = (inputs[i_high][j_high] - inputs[i_low][j_low]) * fs_in;
//                y[n] = inputs[i_low][j_low] + m*(t_out - t_in);
//            }
//
//            offset = (((double)Nout)/fs_out + offset) - ((double)N_in)/fs_in;
//
//        } else {
//            // just return zero samples here as processing cannot be done yet
//            for (int n = 0; n < Nout; n++) {
//                y[n] = 0;
//            }
//        }
//        
//        if (numBuffersProcessed < 2*quality+1) {
//            // no need to keep track of the number of buffers processed once it is past this because the resampler will be fully loaded with all relevent previous input buffers, and we probably don't want to overflow numBuffersProcessed if we're doing lots of resample() calls in a row
//            numBuffersProcessed++;
//        }
//    } // end if inited
//}
//
//void Resampler::unsampleLinear(float *x, int Nin, float *y)
//{
//    if (inited) {
//        // shift the prevs inputs and buffer length states
//        for (int i = 2*quality; i >= 1; i--) {
//            //inputs[i] = inputs[i-1];
//            for (int j = 0; j < N_in; j++) {
//                inputs[i][j] = inputs[i-1][j];
//            }
//            shortBuffer[i] = shortBuffer[i-1];
//        }
//        // ... and add the current input
//        for (int n = 0; n < Nin; n++) {
//            inputs[0][n] = x[n];
//        }
//        // mark the new input buffer as short/normal length
//        if (Nin < N_in) {
//            shortBuffer[0] = true;
//            // fill potentially remaining sample with zeros (for when there is a short buffer)
//            for (int n = Nin; n < N_in; n++) {
//                inputs[0][n] = 0;
//            }
//        } else {
//            shortBuffer[0] = false;
//        }
//        
//        // number of output samples for y
//        int Nout = N_out;
//
//        if (numBuffersProcessed >= quality) {
//            // in this case we have passed the required processing latency so we can compute samples
//            
//            // sample indecies of input buffer samples that boarder each output sample
//            int k_low, k_high;
//            // current time of each output/input sample, relative time for sinc function displacement
//            double t_out, t_in;
//            // localized slope for linear interpolation
//            double m;
//            // more variables, for indexing into inputs array
//            int i_low, i_high, j_low, j_high;
//            
//            for (int n = 0; n < Nout; n++) {
//                // compute shit
//                t_out = ((double)n)/fs_out + offset;
//                k_low = floor(t_out*fs_in);
//                k_high = k_low + 1;
//                t_in = ((double)k_low)/fs_in;
//                
//                // i indexes into which prev buffer we're needing, set it to the middle and adjust
//                i_low = quality;
//                i_high = quality;
//                // j is the indexing variable for the samples, so put it in range [0, Nin-1]
//                j_low = k_low;
//                j_high = k_high;
//                // adjust to proper indexing
//                while (j_low < 0) {
//                    i_low++;
//                    shortBuffer[i_low] ? (j_low += N_in-1) : (j_low += N_in);
//                }
//                while (shortBuffer[i_low] ? (j_low >= N_in-1) : (j_low >= N_in)) {
//                    shortBuffer[i_low] ? (j_low -= N_in-1) : (j_low -= N_in);
//                    i_low--;
//                }
//                while (j_high < 0) {
//                    i_high++;
//                    shortBuffer[i_high] ? (j_high += N_in-1) : (j_high += N_in);
//                }
//                while (shortBuffer[i_high] ? (j_high >= N_in-1) : (j_high >= N_in)) {
//                    shortBuffer[i_high] ? (j_high -= N_in-1) : (j_high -= N_in);
//                    i_high--;
//                }
//                
//                // y = mx + b
//                m = (inputs[i_high][j_high] - inputs[i_low][j_low]) * fs_in;
//                y[n] = inputs[i_low][j_low] + m*(t_out - t_in);
//            }
//            
//            // need to update offset after processing the buffer
//            if (shortBuffer[quality]) {
//                offset = (((double)Nout)/fs_out + offset) - ((double)(N_in-1))/fs_in;
//            } else {
//                offset = (((double)Nout)/fs_out + offset) - ((double)N_in)/fs_in;
//            }
//            
//        } else {
//            // just return zero samples here as processing cannot be done yet
//            for (int n = 0; n < Nout; n++) {
//                y[n] = 0;
//            }
//        }
//        
//        if (numBuffersProcessed < 2*quality+1) {
//            // no need to keep track of the number of buffers processed once it is past this because the resampler will be fully loaded with all relevent previous input buffers, and we probably don't want to overflow numBuffersProcessed if we're doing lots of resample() calls in a row
//            numBuffersProcessed++;
//        }
//    } // end if inited
//}
//
//double Resampler::getFsIn()
//{
//    return fs_in;
//}
//
//double Resampler::getFsOut()
//{
//    return fs_out;
//}
//
//int Resampler::getNin()
//{
//    return N_in;
//}
//
//int Resampler::getNout()
//{
//    if (dir == 0) {
//        // resampling, resamplers have variable buffer size due to the fractional nature of fs_in/fs_out
//        if (shortBuffer[0]) {
//            return floor(N_out);
//        } else {
//            return ceil(N_out);
//        }
//    } else {
//        // unsampling, unsamplers should always output the same number of samples per buffer as was initially fed to the associated resampler
//        return ((int)N_out);
//    }
//}
//
//// returns the maximum size of the output buffer (used for allocating the output buffer's memory)
//int Resampler::getNoutMax()
//{
//    if (dir == 0) {
//        return ceil(N_out);
//    } else {
//        return ((int)N_out);
//    }
//}
//
//int Resampler::getNumBuffersLatency()
//{
//    return quality;
//}

//// not working... resamples N_in samples in x at fs_in and spits out floor(N_out) samples in y at fs_out
//void Resampler::resample(float *x, float *y)
//{
//    if (inited) {
//    // shift the prevs inputs
//    for (int i = 2*quality; i >= 1; i--) {
//        // not sure if we can get away with this...
//        inputs[i] = inputs[i-1];
//        //        for (int j = 0; N_in; j++) {
//        //            inputs[i][j] = inputs[i-1][j];
//        //        }
//    }
//    // ... and add the current input
//    for (int n = 0; n < N_in; n++) {
//        inputs[0][n] = x[n];
//    }
//    
//    // number of output samples for y
//    int Nout;
//    if (dir == 0) {
//        // resampling
//        Nout = ceil(N_out);
//    } else {
//        // unsampling
//        Nout = floor(N_out);
//    }
//    
//    if (numBuffersProcessed >= quality) {
//        // in this case we have passed the required processing latency so we can compute samples
//        
//        // sample indecies of input buffer samples that boarder each output sample
//        int k_low, k_high;
//        // begining and ending indicies of input buffer for the summation when computing a particular output sample
//        int k_begin, k_end;
//        // current time of each output/input sample, relative time for sinc function displacement
//        double t_out, t_in, t;
//        // the minimum of the two sample rates that determines the cuttoff for the low pass filter
//        double fs_min = fmin(fs_in, fs_out);
//        // a thing
//        double thing;
//        // more variables, for indexing into inputs array
//        int i, j;
//        
//        for (int n = 0; n < Nout; n++) {
//            // zero each output sample before summing
//            y[n] = 0;
//            
//            // compute shit
//            t_out = ((double)n)/fs_out + offset;
//            k_low = floor(t_out*fs_in);
//            k_high = k_low + 1;
//            //            // we will get a seg fault later if this is not fixed...
//            //            if (k_high > N_in-1) {
//            //                k_high = N_in-1;
//            //            }
//            k_begin = k_low - 30;//quality*N_in;
//            k_end = k_high + 30;//quality*N_in;
//            
//            for (int k = k_begin; k <= k_end; k++) {
//                t_in = ((double)k)/fs_in;
//                t = t_in - t_out;
//                thing = M_PI*fs_min*t;
//                
//                // i indexes into which prev buffer we're needing, set it to the middle and adjust
//                i = quality;
//                // j is the indexing variable for the samples, so put it in range [0, N_in-1]
//                j = k;
//                while (j < 0) {
//                    j += N_in;
//                    i++;
//                }
//                while (j >= N_in) {
//                    j -= N_in;
//                    i--;
//                }
//                
//                if (thing == 0) {
//                    // divide by zero, bad (sinc is just 1 here anyways)
//                    y[n] += inputs[i][j];
//                } else {
//                    // NOTE: this is lacking an explicit window function (so rectangular window essentially)
//                    // otherwise it is safe to evaluate the sinc function
//                    y[n] += sin(thing)/thing * inputs[i][j];
//                }
//            }
//        }
//        // need to update offset after processing the buffer
//        //offset = 1.0/fs_out - (((double)N_in)/fs_in - t_out);
//        //offset += 1.0/fs_out * (1.0-(N_out-Nout));
//        offset += 1.0/fs_out * (((double)Nout)-N_out);
//        
//        while (offset >= 1.0/fs_out) {
//            offset -= 1.0/fs_out;
//        }
//        while (offset <= -1.0/fs_out) {
//            offset += 1.0/fs_out;
//        }
//        
//    } else {
//        // just return zero samples here as processing cannot be done yet
//        for (int n = 0; n < Nout; n++) {
//            y[n] = 0;
//        }
//    }
//    
//    if (numBuffersProcessed < 2*quality+1) {
//        // no need to keep track of the number of buffers processed once it is past this because the resampler will be fully loaded with all relevent previous input buffers, and we probably don't want to overflow numBuffersProcessed if we're doing lots of resample() calls in a row
//        numBuffersProcessed++;
//    }
//    } // end if inited
//}