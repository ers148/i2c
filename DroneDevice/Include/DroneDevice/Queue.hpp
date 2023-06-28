//
// Queue.hpp
//
//  Created on: Mar 22, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_QUEUE_HPP_
#define DRONEDEVICE_QUEUE_HPP_

#include <cassert>
#include <cstddef>
#include <cstdint>

template<typename T, size_t capacity>
class Queue {
public:
	Queue()
	{
	}

	void clear()
	{
		head = tail = elements = 0;
	}

	bool empty() const
	{
		return elements == 0;
	}

	bool full() const
	{
		return elements == capacity;
	}

	size_t size() const
	{
		return elements;
	}

	T &front()
	{
		assert(elements);
		return data[head];
	}

	void push(T const &aValue)
	{
		assert(elements < capacity);

		data[tail++] = aValue;
		if (tail == capacity) {
			tail = 0;
		}
		++elements;
	}

	size_t push(const T *aBuffer, size_t aLength)
	{
		size_t num = 0;

		while (elements < capacity && aLength--) {
			push(*aBuffer++);
			++num;
		}

		return num;
	}

	size_t pop(T *aBuffer, size_t aLength)
	{
		size_t num = 0;

		while (elements > 0 && num < aLength) {
			*aBuffer++ = pop();
			++num;
		}

		return num;
	}

	T pop()
	{
		assert(elements);

		const T tmp = data[head++];
		if (head == capacity) {
			head = 0;
		}
		--elements;
		return tmp;
	}

protected:
	T data[capacity];

	size_t elements{0}; //!< Current number of elements in the queue
	size_t head{0}; //!< Index of the first element
	size_t tail{0}; //!< Index of the last element
};

#endif // DRONEDEVICE_QUEUE_HPP_
