//
// WorkQueue.hpp
//
//  Created on: Dec 14, 2021
//      Author: Alexander
//

#ifndef PLATFORM_CORTEX_M_WORKQUEUE_HPP_
#define PLATFORM_CORTEX_M_WORKQUEUE_HPP_

#include "Irq.hpp"
#include <DroneDevice/Queue.hpp>
#include <cassert>
#include <functional>

template<size_t size>
class WorkQueue {
public:
	WorkQueue(std::function<void ()> aIdleTask = nullptr) :
		idle{aIdleTask}
	{
	}

	bool add(std::function<void ()> aTask)
	{
		bool result = false;
		irqDisable();

		if (!tasks.full()) {
			tasks.push(aTask);
			result = true;
		}

		irqEnable();
		return result;
	}

	void run()
	{
		while (true) {
#ifdef NDEBUG
			if (idle != nullptr) {
				idle();
			} else {
				irqDisable();
				if (tasks.empty()) {
					sleep();
				}
				irqEnable();
			}
#else
			if (tasks.empty()) {
				if (idle != nullptr) {
					idle();
				} else {
					continue;
				}
			}
#endif

			while (!tasks.empty()) {
				irqDisable();
				auto task = tasks.pop();
				irqEnable();

				task();
			}
		}
	}

private:
	Queue<std::function<void ()>, size> tasks;
	std::function<void ()> idle;
};

#endif // PLATFORM_CORTEX_M_WORKQUEUE_HPP_
