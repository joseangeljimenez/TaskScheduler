/*  ============================================================================
    TaskScheduler.h - Simple scheduler library for AVR ATmega MCUs and Arduino
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
    - Periodic execution of a collaborative set of tasks.
    - Each user-defined task,
      * Must be encapsulated in a simple Callable function.
      * Automatically inherits and carries out the calculation of the elapsed
      time and the time to the next execution, measured as ticks and fraction
      of ticks (phase steps).
      * Automatically calls execute_(), where the user-defined logic resides,
      according to its specific frequency/period requirements.
      * Must be initialized with the following arguments,
        - execute: function reference which will be called periodically.
        - first_execution: unsigned number of ticks until the first execution.
        - period: unsigned number of ticks among two consecutive executions.
        - period_ps and phase_steps_per_tick: allows achieving a repetition
        frequency in case the MCU clock frequency is not an integral multiple
        of that target frequency. When provided, the effective repetition
        period is increased by the ratio period_ps / phase_steps_per_tick.
        - enabled: indicates if the task will be ready for execution right
        after it is added to the scheduler and the scheduler is started.
      * Can be enabled or disabled individually, with Enable() and Disable().
    - The scheduler,
      * Automatically configures, starts and stops the AVR ATmega Timer1.
	  * Automatically invokes each task, notifying every elapsed tick.
      * Must be initialized with the following arguments,
        - tasks: array of previously created tasks.
        - cpu_cycles_per_tick: number of CPU cycles that make one "tick".

    ============================================================================
	Revision history:
    - Version 1.00, original code by Jose Angel Jimenez Vadillo, 2014 Sep 08.
    - Version 1.01, modified 2014 Sep 09 by JAJV,
      * Parameter "num_tasks" of Scheduler::Init() is now removed.
      * Parameter "enabled" of Task::Task() has no default value now.
      * Parameter "start" of Scheduler::Init() has no default value now.
      * Minor refactoring of .h and .cpp.
*/

#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H
#include <stdint.h>
#include <util/atomic.h>

// Library version
#define TASK_SCHEDULER_VERSION = "1.0";

// Custom types
typedef uint32_t Ticks;
typedef uint8_t PhaseSteps;
typedef void (*Callable)();

// User-defined task encapsulating a callable function.
class Task {
	friend class Scheduler;
	public:
        // Public constructors: MUST NOT be called from a user defined task.
		Task(Callable execute, Ticks first_execution, Ticks period,
            bool enabled);
		Task(Callable execute, Ticks first_execution, Ticks period,
			PhaseSteps period_ps, PhaseSteps phase_steps_per_tick,
			bool enabled);

        // Reads or modifies the "enabled" status of the task.
		bool IsEnabled() __attribute__((always_inline)) { return enabled_; }
		void Enable() __attribute__((always_inline)) { enabled_ = true; }
		void Disable() __attribute__((always_inline)) { enabled_ = false; }

	private:
        // Called from Scheduler::Tick(), each time a new "tick" is elapsed.
		void Tick();

        // This property is shared via public methods.
        // However, it is thread-safe, as Task::Tick() only reads it.
        bool enabled_;

		// Hidden properties, only accessed from Task::Tick().
		Callable execute_;
		Ticks current_, period_;
		PhaseSteps current_ps_, period_ps_;
		PhaseSteps phase_steps_per_tick_;
};

// Simple scheduler for periodic execution of collaborative tasks.
class Scheduler {
	public:
        // Initializer: MUST NOT be called from a user defined task.
        template <int SIZE>
        __attribute__((always_inline))
        static void Init(const Task (&tasks)[SIZE],
            uint16_t cpu_cycles_per_tick, bool start=true)
        {
            // Atomic block:
            // - Prevents the ISR execution while making changes.
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                tasks_ = (Task*)tasks;
                Scheduler::num_tasks_ = SIZE;
                InitTimerInterrupt(cpu_cycles_per_tick, start);
            }
        }

        // Restarts or stops the Scheduler, with the same initial parameters.
        // Scheduler::Init() must be previously called.
		static void Restart();
		static void Stop();

		// Debugging: simulates a "tick" by simply executing Scheduler::Tick().
		static void TestTick() __attribute__((always_inline)) { Tick(); }

	private:
        // Called from Scheduler::Init() to initialize the Timer1 and ISR.
        static void InitTimerInterrupt(uint16_t cpu_cycles_per_tick, bool start);
        // Called from the Timer1 ISR each time a new "tick" is elapsed.
		static void Tick();

		// Hidden properties, only accessed from Scheduler::Tick().
		static Task* tasks_;
		static uint8_t num_tasks_;
};

#endif