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
