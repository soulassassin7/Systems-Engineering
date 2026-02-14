#include "day25_ThreadPool.h"

ThreadPool::ThreadPool(size_t num_threads): stop(false){
	for(size_t i = 0; i<num_threads; ++i){
		workers.emplace_back([this]{
			while(true){ 
				std::unique_lock<std::mutex> lock(queue_mutex);
				condition.wait(lock,[this]{ if(!tasks.empty() || stop){
								return true;
						   } else{
								return false;
						   	}
				}); // this wait makes the thread sleep
							// making it very efficient
				if(stop && tasks.empty()){
					return;
				}
				std::function<void()> f = tasks.front();
				tasks.pop();
				lock.unlock();
				f(); // execute the function
			}
		});
	}
} /* The lambda [this]{ ... } is the permanent life cycle of the worker thread. 
     Since we don't know what jobs will arrive later, 
     we give the worker a generic instruction manual:

     "Stay alive. Loop forever. Check the queue. If there's a job, do it. If not, sleep." */


ThreadPool::~ThreadPool(){
	{
		std::unique_lock<std::mutex> lock(queue_mutex); // lock_guard would also work
		stop = true;
	}
	condition.notify_all();

	for(std::thread &worker: workers){
		worker.join();
	}
}


void ThreadPool::enqueue(std::function<void()> task){
		std::unique_lock<std::mutex> lock(queue_mutex);
		tasks.push(task);
		lock.unlock();
		condition.notify_one(); // wake up only one worker to handle it
}
