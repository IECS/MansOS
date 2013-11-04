/*
 * Copyright (c) 2008-2012 the MansOS team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of  conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "algo.h"
#include <timing.h>

// Calculate square root, rounded down, exception:
// intSqrt(8) = 3
// Accuracy +-1.
// Highest possible inaccuracy when:
// intSqrt(x^2 - 1) = sqrt(x - 1)
uint16_t intSqrt(uint32_t val)
{
    // Rough estimation of result
    // can be calculated using formulas:
    // d = digit count
    // x0 = 2 * 10^n and n = (d - 1)/2, if d is odd
    // x0 = 6 * 10^n and n = (d - 2)/2, if d is even
    // but since possible values ain't too much, we can hard code them
    uint32_t prev = (val < 10 ? 2 :
        (val < 100 ? 6 :
        (val < 1000 ? 20 :
        (val < 10000 ? 60 :
        (val < 100000 ? 200 :
        (val < 1000000 ? 600 :
        (val < 10000000 ? 2000 :
        (val < 100000000 ? 6000 :
        (val < 1000000000 ? 20000 :
       60000)))))))));
    uint32_t cur = prev, next;
#ifdef DEBUG
    PRINTF("Starting sqrt(%lu) = %lu\n", val, cur);
    uint8_t iter = 1;
#endif // DEBUG
    // Babylonian method for square root calculation
    while(cur)
    {
        // Calculating next estimation
        next = (cur + val / cur) / 2;
        // Stop when next value equals current value.
        // But since we are using integers there might be situation, when next
        // and current value differs by 1 and they switch values each iteration
        // and never are equal, so we need to check equality with last two values
        // and take current value not next, because it yields correct result.
        if (next == cur || next == prev)
        {
#ifdef DEBUG
            PRINTF("    With %d iterations found sqrt(%lu) = %lu\n", iter, val, cur);
#endif // DEBUG
            return cur;
        }

        prev = cur;
        cur = next;
#ifdef DEBUG
        iter++;
        PRINTF("    Itering sqrt(%lu) = %lu\n", val, cur);
#endif // DEBUG
    }
    return 0;
}

//
// Calculate approximate triangle wave value at given point of time
//
uint16_t signalTriangleWave(uint16_t period, uint16_t low, uint16_t high)
{
    const uint16_t amplitude = high - low;
    uint16_t positionX = getJiffies() % period;
    const int32_t saw = (2*amplitude/period)*positionX - amplitude;
    const uint16_t positionY = abs(saw)+low;
    return positionY;
}

//
// Calculate approximate sawtooth wave value at given point of time
//
uint16_t signalSawtoothWave(uint16_t period, uint16_t low, uint16_t high)
{
    const uint16_t amplitude = high - low;
    uint16_t positionX = getJiffies() % period;
    const uint16_t positionY = (positionX * amplitude) / period + low;
    return positionY;
}

//
// Calculate approximate sine wave value at given point of time
//
uint16_t signalSineWave(uint16_t period, uint16_t low, uint16_t high)
{
    const uint16_t halfperiod = period/2;
    const uint16_t halfhalfperiod = halfperiod/2;
    const uint16_t amp = (high - low)/2;
    const uint16_t off = (high + low)/2;
    static uint16_t sin[89];//sin(deg)*4096
    sin[0] = 0;
    sin[1] =71;
    sin[2] =143;
    sin[3] =214;
    sin[4] =286;
    sin[5] =357;
    sin[6] =428;
    sin[7] =499;
    sin[8] =570;
    sin[9] =641;
    sin[10] =711;
    sin[11] =782;
    sin[12] =852;
    sin[13] =921;
    sin[14] =991;
    sin[15] =1060;
    sin[16] =1129;
    sin[17] =1198;
    sin[18] =1266;
    sin[19] =1334;
    sin[20] =1401;
    sin[21] =1468;
    sin[22] =1534;
    sin[23] =1600;
    sin[24] =1666;
    sin[25] =1731;
    sin[26] =1796;
    sin[27] =1860;
    sin[28] =1923;
    sin[29] =1986;
    sin[30] =2048;
    sin[31] =2110;
    sin[32] =2171;
    sin[33] =2231;
    sin[34] =2290;
    sin[35] =2349;
    sin[36] =2408;
    sin[37] =2465;
    sin[38] =2522;
    sin[39] =2578;
    sin[40] =2633;
    sin[41] =2687;
    sin[42] =2741;
    sin[43] =2793;
    sin[44] =2845;
    sin[45] =2896;
    sin[46] =2946;
    sin[47] =2996;
    sin[48] =3044;
    sin[49] =3091;
    sin[50] =3138;
    sin[51] =3183;
    sin[52] =3228;
    sin[53] =3271;
    sin[54] =3314;
    sin[55] =3355;
    sin[56] =3396;
    sin[57] =3435;
    sin[58] =3474;
    sin[59] =3511;
    sin[60] =3547;
    sin[61] =3582;
    sin[62] =3617;
    sin[63] =3650;
    sin[64] =3681;
    sin[65] =3712;
    sin[66] =3742;
    sin[67] =3770;
    sin[68] =3798;
    sin[69] =3824;
    sin[70] =3849;
    sin[71] =3873;
    sin[72] =3896;
    sin[73] =3917;
    sin[74] =3937;
    sin[75] =3956;
    sin[76] =3974;
    sin[77] =3991;
    sin[78] =4006;
    sin[79] =4021;
    sin[80] =4034;
    sin[81] =4046;
    sin[82] =4056;
    sin[83] =4065;
    sin[84] =4074;
    sin[85] =4080;
    sin[86] =4086;
    sin[87] =4090;
    sin[88] =4094;
    sin[89] =4095;
    sin[90] =4096;
    int32_t val = getJiffies() % period;
	bool invert =0;
	if(val > halfperiod)
	{
		val = val - halfperiod;
		invert = 1;
	}
	if(val > halfhalfperiod)
	{
		val = abs(val - halfperiod);
	}
	val = 90*val/halfhalfperiod;
	val = sin[val];
    if(invert)
    {
		val = off - amp*val/4096;
    }
    if(!invert)
    {      
		val = off + amp*val/4096;
    }
    return val;
}
