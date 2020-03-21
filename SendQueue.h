#pragma once

#include <string>
#include <atomic>

class SendQueue
{
	struct SendData
	{
		std::string data;
		SendData* next;
	};

	std::atomic<SendData*> head = nullptr;

public:
	void push(const std::string& str) {
		auto new_head = new SendData{ str, nullptr };
		SendData* old_head = head.load();
		do
			new_head->next = old_head;
		while (!head.compare_exchange_weak(old_head, new_head));
	}

	template<class OutputIter>
	void move_all(OutputIter out)
	{
		SendData* cur_head = head.exchange(nullptr);
		while (cur_head != nullptr) {
			*out++ = std::move(cur_head->data);
			delete std::exchange(cur_head, cur_head->next);
		}
	}
};