/*  ============================================================================
    TimerInterrupt.h - Library for the Timer1 of AVR ATmega MCUs and Arduino  
	============================================================================
    Copyright 2014 by Jose Angel Jimenez Vadillo, who supports and encourages
    the free dissemination and exchange of information,
    http://linkedin.com/in/joseangeljimenez/

    This is open software. The copyright of this source code is licensed under
    "Creative Commons Attribution-ShareAlike 4.0 International" (CC BY-SA 4.0).

    For more information on this license, visit
    http://creativecommons.org/licenses/by-sa/4.0/

    You are free to share and adapt this work, under the following terms:
    - Attribution — You must give appropriate credit, provide a link to the
    license, and indicate if changes were made. You may do so in any
    reasonable manner, but not in any way that suggests the licensor endorses
    you or your use.
    - ShareAlike — If you remix, transform, or build upon the material, you
    must distribute your contributions under the same license as the original.

	The author of this work shall not be liable for technical or other errors
	or omissions contained herein. You accept all risks and responsibility for
	losses, damages, costs and other consequences resulting directly or
	indirectly from using this work.

    ============================================================================
    Features:
    - Configures the Timer1 for the indicated cpu_cycles_period.
    - [To be documented]

    ============================================================================
	Revision history:
    - Original code by Jose Angel Jimenez Vadillo, 2014 Sep 08.
    - Modified [yyyy month] by [xxx] and [yyy] to [zzz].
*/

#ifndef TIMER_INTERRUPT_H
#define TIMER_INTERRUPT_H
#include <util/atomic.h>
#include <avr/interrupt.h>
#include "TaskScheduler.h"

// Library version
#define TIMER_INTERRUPT_VERSION = "1.0";

// ISR for Timer1 overflow (tick).
ISR(TIMER1_OVF_vect);

__attribute__((always_inline))
static bool CheckPrescaler(uint16_t cycles, uint16_t prescaler_value) {
	return (cycles >= 4 * prescaler_value) && ((cycles & prescaler_value - 1) == 0);
}

class TimerInterrupt {
	friend void TIMER1_OVF_vect();
	public:
		// The minimum value allowed for cpu_cycles_period is 4.
		__attribute__((always_inline))
		static void Init(Callable isr_callback,
			uint16_t cpu_cycles_period=256, bool start=true)
		{
			// Atomic block:
			// - Prevents the ISR execution while making changes.
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				Attach(isr_callback);
				uint16_t cycles = cpu_cycles_period;

				// Selects the highest prescaler while ensuring top_value_ >= 3.
				if (CheckPrescaler(cycles, 1024)) {
					// Prescaler:  set to clkIO/1024.
					prescaler_bits_ = _BV(CS12) | _BV(CS10);
					top_value_ = cycles / 1024 - 1;
				}
				else if (CheckPrescaler(cycles, 256)) {
					// Prescaler set to clkIO/256.
					prescaler_bits_ = _BV(CS12);
					top_value_ = cycles / 256 - 1;
				}
				else if (CheckPrescaler(cycles, 64)) {
					// Prescaler set to clkIO/64.
					prescaler_bits_ = _BV(CS11) | _BV(CS10);
					top_value_ = cycles / 64 - 1;
				}
				else if (CheckPrescaler(cycles, 8)) {
					// Prescaler set to clkIO/8.
					prescaler_bits_ = _BV(CS11);
					top_value_ = cycles / 8 - 1;
				}
				else {
					// Prescaler set to clkIO/1 (no prescaling).
					prescaler_bits_ = _BV(CS10);
					top_value_ = cycles - 1;
				}
				if (start)
				{
					Restart();
				}
			}
		}

		static void Restart();
		static void Stop();
		static void Attach(Callable isr_callback);

	private:
		// Prescaler bits configuration (1, 1/8, 1/64, 1/256, 1/1024).
		static uint8_t prescaler_bits_;
		// The timer counts from 0 to TOP, then restarts.
		static uint16_t top_value_;
		// Reference to the ISR callback function.
		static Callable isr_callback_;
};

#endif