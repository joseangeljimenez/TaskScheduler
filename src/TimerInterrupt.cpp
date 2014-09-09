/*  ============================================================================
    TimerInterrupt.cpp - Library for the Timer1 of AVR ATmega MCUs and Arduino  
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
	Revision history:
    - Original code by Jose Angel Jimenez Vadillo, 2014 Sep 08.
    - Modified [yyyy month] by [xxx] and [yyy] to [zzz].
*/

#include "TimerInterrupt.h"
#include <util/atomic.h>
#include <avr/interrupt.h>
#include "TaskScheduler.h"

// Private properties.
uint8_t TimerInterrupt::prescaler_bits_;
uint16_t TimerInterrupt::top_value_;
Callable TimerInterrupt::isr_callback_;

void TimerInterrupt::Restart() {
	Stop();
	// Timer1 configuration update:
	// - Disconnects OC1A/B output compare pins.
	// - Sets mode to Fast PWM with TOP=ICR1.
	// - Enables and sets the clock/prescaler.
	TCCR1A = _BV(WGM11);
	TCCR1B = _BV(WGM13) | _BV(WGM12) | prescaler_bits_;
	// - Enables TOIE1 (Timer1 Overflow Interrupt Enable);
	TIMSK1 |= _BV(TOIE1);
}

void TimerInterrupt::Stop() {
	// Atomic block:
	// - Protects ICR1/TCNT1 (16-bit registers) from shared access.
	// - ICR1/TCNT1 update could trigger an interrupt.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Timer1 configuration update:
		// - Disables the clock/prescaler, which stops the counter.
		TCCR1B = 0;
		// - Sets the TOP value and clears the counter.
		ICR1 = top_value_;
		TCNT1 = 0;
		// - Clears all pending interrupts (just in case).
		TIFR1 = 0xFF;
		// - Disables TOIE1 (Timer1 Overflow Interrupt Enable);
		TIMSK1 = _BV(TOIE1);
	}
}

void TimerInterrupt::Attach(Callable isr_callback) {
	// Atomic block:
	// - Prevents the ISR execution while updating the callback reference.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		isr_callback_ = isr_callback;
	}
}

// ISR for Timer1 overflow (tick).
ISR(TIMER1_OVF_vect)
{
	TimerInterrupt::isr_callback_();
}