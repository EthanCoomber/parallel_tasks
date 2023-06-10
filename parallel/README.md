## Getting Started

After compiling with "cmake3 ." the first time, typing "make" into the command line when in the parallel folder will compile the code.

## Introduction

The ISPC task system relies on a runtime that can execute the tasks, i.e., create tasks and assign them to different threads for execution. In this code, C++ threads and mechanisms for implementing data parallelism on shared memory systems are on display.

## Implementation

The main program for this code can be compiled with `make pa2-main`. The `pa2-main` program will run various benchmarks using the different task runners (use the `-h` option to see the available options). You can change the number of threads with the `-t` option and specify a single test to run with the `-n` option (to list the available tests use the `-l` option, i.e. `./pa2-main -l`). For example the following would run a single benchmark named `SuperLightTest` with 4 threads.

```
./pa2-main -n SuperLightTest -t 4
Test [4 threads]: SuperLightTest
[TaskRunnerSerial]:             61.553 ms
[TaskRunnerSpawn]:              55.300 ms
[TaskRunnerSpin]:               40.761 ms
[TaskRunnerSleep]:              36.673 ms
```

## How I implemented the task runners and assigned work ensuring all work was completed before returning.

With the TaskRunnerSpawn, I took an approach similar to Mandelbrot from assignment 1. I assigned threads statically where each thread took on several tasks. This way, I was able to ensure that all threads were working at the same time. With TaskRunnerSpin, tasks were assigned dynamically. This is because the threads were "spinning" and waiting to be called up. Only when one task completed, did I look to assign the next task and pull in a spinner. I had to ensure that the threads were locking when I needed them to so they could perform the necessary task. I originally tried to use a for loop to assign tasks but later realized it would be more efficient to use a while loop and be checking to make sure there was still work to do. I came across an error though as my program was finishing before all work was done. This was because some threads were still doing work while the program finished. I had to add another variable that would be used to ensure the program would wait for all the work to finish. With TaskRunnerSleep, I took a similar approach to task runner spin where I am dynamically assigning tasks however, I am putting the threads to sleep when they are not being used. I had to be careful with this approach as to ensure that the threads were awake when I needed them to be and also sleeping whenever they were not immediately being used.