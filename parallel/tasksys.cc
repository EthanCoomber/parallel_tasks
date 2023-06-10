#include "tasksys.h"
#include <atomic>
#include <cassert>
#include <thread>

void TaskRunnerSerial::Run(Runnable* runnable, int num_tasks) {
  for (int i = 0; i < num_tasks; i++) {
    runnable->RunTask(i, num_tasks);
  }
}

void spawnHelper(Runnable* runnable, int thread_index, int num_tasks, int tasksPerThread){
  int start = thread_index*tasksPerThread;
  int end = std::min(start+tasksPerThread, num_tasks);
  for(int i = start; i < end; i++){
    runnable->RunTask(i, num_tasks);
  }
  
}


void TaskRunnerSpawn::Run(Runnable* runnable, int num_tasks) {

  std::thread spawn[num_threads_];

  int tasksPerThread = (num_tasks + num_threads_ - 1) / num_threads_;
  for (int i = 0; i < num_threads_; i++) {
    
    spawn[i] = std::thread(spawnHelper, runnable, i, num_tasks, tasksPerThread);
  }

  for(int i = 0; i < num_threads_; i++) {
    spawn[i].join();
  }
}


void TaskRunnerSpin::RunningThread(){
  while (running_.load()) {
    sync.lock();
    while (next_task < num_tasks) {
      int local_task = next_task++;
      sync.unlock();
      runnable->RunTask(local_task, num_tasks);
      sync.lock();
      tasks_done++;
    }
    sync.unlock();
  }
}

void TaskRunnerSpin::Run(Runnable* runnable, int num_tasks) {
  sync.lock();
  this->runnable = runnable;
  this->next_task = 0;
  this->num_tasks = num_tasks;
  this->tasks_done = 0;
  sync.unlock();
  
  sync.lock();
  while (next_task < num_tasks) {
    int local_task = next_task++;
    sync.unlock();
    runnable->RunTask(local_task, num_tasks);
    sync.lock();
    tasks_done++;
  }
  sync.unlock();
  
  sync.lock();
  while(tasks_done < num_tasks){
    sync.unlock();
    sync.lock();
  }
  sync.unlock();
}

TaskRunnerSpin::TaskRunnerSpin(int num_threads) : num_threads_(num_threads), running_(true), next_task(0), num_tasks(0) {
  // Launch threads
  threads_.resize(num_threads_);
  for (int i = 1; i < num_threads_; ++i) {
    threads_[i] = std::thread(&TaskRunnerSpin::RunningThread, this);
  }
  
}

TaskRunnerSpin::~TaskRunnerSpin() {
  // Join
  running_ = false;
  for(int i = 1; i < num_threads_; i++) {
    threads_[i].join();
  }
}

void TaskRunnerSleep::RunningThread(){
      std::unique_lock<std::mutex> lock(sync);
      while (running_.load()) {
        //std::unique_lock<std::mutex> lock(sync);
        if (next_task >= num_tasks)
          cond_.wait(lock);
        
        while (next_task < num_tasks) {
          int local_task = next_task++;
          lock.unlock();
          
          runnable->RunTask(local_task, num_tasks);
          lock.lock();
          tasks_done++;
        }
      }
}

void TaskRunnerSleep::Run(Runnable* runnable, int num_tasks) {

  sync.lock();
  this->runnable = runnable;
  this->next_task = 0;
  this->num_tasks = num_tasks;
  this->tasks_done = 0;
  cond_.notify_all();
  sync.unlock();

  sync.lock();
  while (next_task < num_tasks) {
    int local_task = next_task++;
    sync.unlock();
    
    runnable->RunTask(local_task, num_tasks);
    sync.lock();
    tasks_done++;
  }
  sync.unlock();

  sync.lock();
  while(tasks_done < num_tasks){
    sync.unlock();
    sync.lock();
  }
  sync.unlock();
}

TaskRunnerSleep::TaskRunnerSleep(int num_threads) : num_threads_(num_threads), running_(true), next_task(0), num_tasks(0) {
  threads_.resize(num_threads_);
  for (int i = 1; i < num_threads_; ++i) {
    threads_[i] = std::thread(&TaskRunnerSleep::RunningThread, this);
  }
}

TaskRunnerSleep::~TaskRunnerSleep() {
  running_ = false;
  cond_.notify_all();
  for(int i = 1; i < num_threads_; i++) {
    threads_[i].join();
  }
}

