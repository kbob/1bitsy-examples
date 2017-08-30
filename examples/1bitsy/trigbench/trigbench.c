/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/stm32/rcc.h>

#include "console.h"
#include "gpio.h"
#include "systick.h"

#define KINETISK // Hack: means Cortex-M4
#include "dspinst.h"
#undef KINETISK

#define CLOCK_SCALE (rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ])

#define NREP 997

struct stats {
    uint32_t run_no;
    uint32_t libm_cyccnt;
    float libm_avg_cycles;
    uint32_t int_cyccnt;
    float int_avg_cycles;
    uint32_t float_cyccnt;
    float float_avg_cycles;
};

volatile struct stats stats;

volatile float libm_sin_args[NREP];
volatile float libm_sin_results[NREP];
volatile uint32_t int_sin_args[NREP];
volatile int32_t int_sin_results[NREP];
volatile float float_sin_args[NREP];
volatile float float_sin_results[NREP];

static void setup_clocks(void)
{
    rcc_clock_setup_hse_3v3(&CLOCK_SCALE);
}


static void setup_LEDs(void)
{
    static const gpio_pin LED_pins[] = {
        {                       // on-board LED: PA8.
            .gp_port   = GPIOA,
            .gp_pin    = GPIO8,
            .gp_mode   = GPIO_MODE_OUTPUT,
            .gp_pupd   = GPIO_PUPD_NONE,
            .gp_af     = GPIO_AF0,
            .gp_ospeed = GPIO_OSPEED_DEFAULT,
            .gp_otype  = GPIO_OTYPE_PP,
            .gp_level  = 1, // N.B., the on-board LED lights when signal is LOW.
        },
    };
    static const size_t LED_pin_count = (&LED_pins)[1] - LED_pins;
        
   gpio_init_pins(LED_pins, LED_pin_count);
}

static void setup_constants(void)
{
    for (size_t i = 0; i < NREP; i++)
        libm_sin_args[i] = 2 * i * M_PI / NREP;
}

// From
// https://dorkbotpdx.org/blog/paul/generating_high_precision_sine_wave_data
//
// High accuracy 11th order Taylor Series Approximation
// input is 0 to 0xFFFFFFFF, representing 0 to 360 degree phase
// output is 32 bit signed integer, top 25 bits should be very good
static int32_t taylor(uint32_t ph)
{
        int32_t angle, sum, p1, p2, p3, p5, p7, p9, p11;

        if (ph >= 0xC0000000 || ph < 0x40000000) {
                angle = (int32_t)ph; // valid from -90 to +90 degrees
        } else {
                angle = (int32_t)(0x80000000u - ph);
        }
        p1 =  multiply_32x32_rshift32_rounded(angle << 1, 1686629713);
        p2 =  multiply_32x32_rshift32_rounded(p1, p1) << 3;
        p3 =  multiply_32x32_rshift32_rounded(p2, p1) << 3;
        sum = multiply_subtract_32x32_rshift32_rounded(p1 << 1, p3, 1431655765);
        p5 =  multiply_32x32_rshift32_rounded(p3, p2) << 1;
        sum = multiply_accumulate_32x32_rshift32_rounded(sum, p5, 286331153);
        p7 =  multiply_32x32_rshift32_rounded(p5, p2);
        sum = multiply_subtract_32x32_rshift32_rounded(sum, p7, 54539267);
        p9 =  multiply_32x32_rshift32_rounded(p7, p2);
        sum = multiply_accumulate_32x32_rshift32_rounded(sum, p9, 6059919);
        p11 = multiply_32x32_rshift32_rounded(p9, p2);
        sum = multiply_subtract_32x32_rshift32_rounded(sum, p11, 440721);
        return sum <<= 1;
}

static float taylor_sinf(float x)
{
    const float coeff1 = -0x1.555556p-3;
    const float coeff2 = +0x1.111112p-7;
    const float coeff3 = -0x1.a01a02p-13;
    const float coeff4 = +0x1.71de3ap-19;
    const float coeff5 = -0x1.ae6456p-26;
    const float coeff6 = +0x1.612462p-33;

    float y = 0;
    float x2 = x * x;
    float xp = x;
    y += xp;
    xp *= x2;
    y += xp * coeff1;
    xp *= x2;
    y += xp * coeff2;
    xp *= x2;
    y += xp * coeff3;
    xp *= x2;
    y += xp * coeff4;
    xp *= x2;
    y += xp * coeff5;
    xp *= x2;
    y += xp * coeff6;
    xp *= x2;
    return y;
}

int main(void)
{
    setup_clocks();
    setup_systick(CLOCK_SCALE.ahb_frequency);
    setup_console();
    setup_console_stdio();
    setup_LEDs();
    setup_constants();

    ((SCS_DEMCR |= SCS_DEMCR_TRCENA),
     (SCS_DWT_CTRL |= SCS_DWT_CTRL_CYCCNTENA));
    delay_msec(1000);

    while (true) {

        gpio_clear(GPIOA, GPIO8); // LED on

        // Test libm sin.
        {
            uint32_t b = SCS_DWT_CYCCNT;
            for (size_t i = 0; i < NREP; i++)
                libm_sin_results[i] = sinf(libm_sin_args[i]);
            uint32_t a = SCS_DWT_CYCCNT;

            stats.libm_cyccnt = a - b;
            stats.libm_avg_cycles = (float)(a - b) / (float)NREP;
        }

        // Test Paul's integer sin.
        {
            uint32_t b = SCS_DWT_CYCCNT;
            for (size_t i = 0; i < NREP; i++)
                int_sin_results[i] = taylor(int_sin_args[i]);
            uint32_t a = SCS_DWT_CYCCNT;

            stats.int_cyccnt = a - b;
            stats.int_avg_cycles = (float)(a - b) / (float)NREP;
        }

        // Test float sin.
        {
            uint32_t b = SCS_DWT_CYCCNT;
            for (size_t i = 0; i < NREP; i++)
                float_sin_results[i] = taylor_sinf(float_sin_args[i]);
            uint32_t a = SCS_DWT_CYCCNT;

            stats.float_cyccnt = a - b;
            stats.float_avg_cycles = (float)(a - b) / (float)NREP;
        }

        stats.run_no++;
        gpio_set(GPIOA, GPIO8); // LED off

        delay_msec(2000);
    }

    return 0;
}
