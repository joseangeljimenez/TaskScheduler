/*  ============================================================================
    TaskScheduler.cpp - Simple scheduler library for AVR ATmega MCUs and Arduino
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

#include <util/atomic.h>
#include "TimerInterrupt.h"
#include "TaskScheduler.h"

Task::Task(Callable execute, Ticks first_execution, Ticks period,
    bool enabled):
    // Properties initialization.
    execute_(execute), current_(first_execution), period_(period),
    current_ps_(0), period_ps_(0), phase_steps_per_tick_(0),
    enabled_(enabled) {
}

Task::Task(Callable execute, Ticks first_execution, Ticks period,
    PhaseSteps period_ps, PhaseSteps phase_steps_per_tick, bool enabled):
    // Properties initialization.
    execute_(execute), current_(first_execution), period_(period),
    current_ps_(0), period_ps_(period_ps),
    phase_steps_per_tick_(phase_steps_per_tick), enabled_(enabled) {
}

// Called from Scheduler::Tick() and ISR(TIMER1_OVF_vect)
void Task::Tick() {
    if (current_-- == 0) {
        current_ += period_;
        if (phase_steps_per_tick_ > 0) {
            current_ps_ += period_ps_;
            if (current_ps_ >= phase_steps_per_tick_) {
                current_ps_ -= phase_steps_per_tick_;
                current_ += 1;
            }
        }
        if (enabled_) {
            execute_();
        }
    }
}

// Private properties.
Task* Scheduler::tasks_;
uint8_t Scheduler::num_tasks_;

// This function MUST NOT be called from the ISR (user defined task).
void Scheduler::Init(const Task tasks[], uint8_t num_tasks,
    uint16_t cpu_cycles_per_tick, bool start)
{
    // Atomic block:
    // - Avoids inconsistent reads of tasks_ and num_tasks_ from an ISR.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        tasks_ = (Task*) tasks;
        num_tasks_ = num_tasks;
        TimerInterrupt::Init(Scheduler::Tick, cpu_cycles_per_tick, start);
    }
}

void Scheduler::Restart() {
    TimerInterrupt::Restart();
}

void Scheduler::Stop() {
    TimerInterrupt::Stop();
}

// Called from ISR(TIMER1_OVF_vect)
// Defining SLOWDOWN as true makes "ticking" 256 times slower (debugging).
#define SLOWDOWN false
void Scheduler::Tick() {
    static uint8_t slowdown = 0;
    if (SLOWDOWN) ++slowdown;
    if (!slowdown)
    {
        for(int i = 0; i < num_tasks_; ++i) {
            tasks_[i].Tick();
        }
    }
}