#pragma once
#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

/**
 * @brief Abstract base case class for all tasks
 */
class Runnable {
 public:
  virtual ~Runnable() {}

  /**
   * @brief Method invoked by task runner
   * 
   * @param task_id Unique ID for for current task
   * @param task_count Total number of tasks to be executed
   */
  virtual void RunTask(int task_id, int task_count) = 0;
};

/**
 * @brief Abstract base class for task runners
 */
class TaskRunner {
 public:
  virtual ~TaskRunner() {}

  /**
   * @brief Bulk launch tasks
   * @param runnable Task to execute
   * @param num_tasks Number of tasks to launch
   */
  virtual void Run(Runnable* runnable, int num_tasks) = 0;
};

class TaskRunnerSerial : public TaskRunner {
 public:
  void Run(Runnable* runnable, int num_tasks) override;
};

class TaskRunnerSpawn : public TaskRunner {
 public:
  TaskRunnerSpawn(int num_threads) : num_threads_(num_threads) {}

  void Run(Runnable* runnable, int num_tasks) override;

 private:
  int num_threads_;
};

class TaskRunnerSpin : public TaskRunner {
 public:
  TaskRunnerSpin(int num_threads);
  ~TaskRunnerSpin();

  void Run(Runnable* runnable, int num_tasks) override;

 private:
  int num_threads_;
  std::vector<std::thread> threads_;
  std::atomic<bool> running_;  
  std::mutex sync;
  int next_task;
  int num_tasks;
  int tasks_done;
  Runnable* runnable;

  void RunningThread();
};

class TaskRunnerSleep : public TaskRunner {
 public:
  TaskRunnerSleep(int num_threads);
  ~TaskRunnerSleep();

  void Run(Runnable* runnable, int num_tasks) override;

 private:
  int num_threads_;
  int next_task;
  int tasks_done;
  int num_tasks;
  std::mutex sync;
  std::condition_variable cond_;
  std::atomic<bool> running_;  
  std::vector<std::thread> threads_;
  Runnable* runnable;

  void RunningThread();
};
