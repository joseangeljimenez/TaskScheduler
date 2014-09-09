TaskScheduler
=============
This is a simple scheduler library for AVR ATmega MCUs and Arduino.

Features
--------
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

Installation
------------
For installing this library in an Arduino environment, download the zip package
from the "download" folder, run the Arduino IDE and import the library with the
"Import Library > Add Library option", inside the appropriate drop-down menu.

More detailed instructions can be found here,

http://arduino.cc/en/Guide/Libraries

Example
-------
A simple sample sketch is included in the "examples" folder.

```C++
#include <TaskScheduler.h>

// Configuration
#define CPU_FREQUENCY 16e6
#define CPU_CYCLES_PER_TICK 256
#define TICKS_PER_SECOND (CPU_FREQUENCY / CPU_CYCLES_PER_TICK)

// Function f1 simply counts the number of elapsed seconds.
void f1(){
  static uint32_t seconds = 0;
  Serial.println(seconds++);
}

// Function f2 toggles the LED on pin13 each 500ms.
void f2(){
  static uint8_t led = HIGH;
  led = HIGH ? (led == LOW) : LOW;
  digitalWrite(13, led);
}

Task t1(f1, 0, TICKS_PER_SECOND, true);
Task t2(f2, 0, 0.5 * TICKS_PER_SECOND, true);
const Task tasks[] = { t1, t2 };

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  Scheduler::Init(tasks, CPU_CYCLES_PER_TICK, true);
}

void loop() {
}
```

Revision history
----------------
- Version 1.00, original code by Jose Angel Jimenez Vadillo, 2014 Sep 08.
- Version 1.01, modified 2014 Sep 09 by JAJV,
  * Parameter "num_tasks" of Scheduler::Init() is now removed.
  * Parameter "enabled" of Task::Task() has no default value now.
  * Parameter "start" of Scheduler::Init() has no default value now.
  * Minor refactoring of .h and .cpp.

License
-------
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