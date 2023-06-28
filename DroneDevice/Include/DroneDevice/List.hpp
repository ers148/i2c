//
// List.hpp
//
//  Created on: Jul 17, 2019
//      Author: Alexander
//

#ifndef DRONEDEVICE_LIST_HPP_
#define DRONEDEVICE_LIST_HPP_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>

template<typename T>
struct ListNode {
	ListNode<T> *next;
	T data;

	T &operator*()
	{
		return data;
	}
};

template<typename T, size_t capacity>
class List {
public:
	using Node = ListNode<T>;

	List()
	{
		clear();
	}

	void clear()
	{
		first = nullptr;
		available = capacity;

		for (size_t i = 0; i < capacity; ++i) {
			pool[i] = &arena[i];
		}
	}

	bool empty() const
	{
		return first == nullptr;
	}

	bool full() const
	{
		return available == 0;
	}

	size_t size() const
	{
		return capacity - available;
	}

	Node *front()
	{
		return first;
	}

	const Node *front() const
	{
		return first;
	}

	void pushBack(T const &aValue)
	{
		assert(available > 0);

		auto node = allocate();
		node->data = aValue;
		node->next = nullptr;

		if (first != nullptr) {
			auto current = first;

			while (current->next) {
				current = current->next;
			}

			current->next = node;
		} else {
			first = node;
		}
	}

	Node *erase(Node *aNode)
	{
		auto node = &first;

		while (*node != nullptr && *node != aNode) {
			node = &(*node)->next;
		}

		if (*node != nullptr) {
			auto target = *node;
			*node = (*node)->next;
			release(target);
		}

		return *node;
	}

	void erase(T const &aValue)
	{
		auto node = &first;

		while (*node != nullptr && ***node != aValue) {
			node = &(*node)->next;
		}

		if (*node != nullptr) {
			auto target = *node;
			*node = (*node)->next;
			release(target);
		}
	}

	void forEach(std::function<void (T const &)> aAction)
	{
		auto node = first;

		while (node != nullptr) {
			aAction(node->data);
			node = node->next;
		}
	}

	Node *findIf(std::function<bool (T const &)> aPredicate)
	{
		auto node = first;

		while (node != nullptr) {
			if (aPredicate(node->data)) {
				return node;
			}
			node = node->next;
		}

		return nullptr;
	}

	void eraseIf(std::function<bool (T const &)> aPredicate)
	{
		auto node = &first;

		while (*node) {
			if (aPredicate((*node)->data)) {
				auto target = *node;

				*node = (*node)->next;
				release(target);
			} else {
				node = &(*node)->next;
			}
		}
	}

protected:
	Node *first;
	std::array<Node, capacity> arena;
	std::array<Node *, capacity> pool;
	size_t available;

	Node *allocate()
	{
		assert(available > 0);
		return pool[--available];
	}

	void release(Node *aElement)
	{
		assert(available < capacity);
		pool[available++] = aElement;
	}
};

#endif // DRONEDEVICE_LIST_HPP_
