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