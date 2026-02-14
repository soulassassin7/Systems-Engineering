#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional> // for std::function
// std::function is a polymorphic function wrapper that can store normal functions
// lambda functions, or functors
// Syntax std::function<ReturnType(Args...)>


class ThreadPool{
private:
	// 1. Workers
	std::vector<std::thread> workers;

	// 2. The task queue
	std::queue<std::function<void()>> tasks;

	//3. Synchronisation
	std::mutex queue_mutex;

	//4. Notification
	std::condition_variable condition;
	// we don't use bool here because it will keep checking constantly
	//consuming 100% cpu eventually, conditional_variable is like a doorbell
	// it notifies us when tasks arrives leaving our cpu usage almost 0%

	//5. Shutdown signal
	bool stop;

public:
	// Constructor
	ThreadPool(size_t num_threads);
	
	// Destructor: fires everyone when server closes
	~ThreadPool();
	
	//Enqueue: post new job on the board
	void enqueue(std::function<void()> task);
};

#endif
