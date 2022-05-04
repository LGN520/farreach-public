#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "helper.h"

template<class obj_t>
class MessagePtrQueue {
	public:
		MessagePtrQueue();
		MessagePtrQueue(uint32_t size);
		void init(uint32_t size);
		~MessagePtrQueue();

		bool write(obj_t * newobj);
		obj_t * read();

		uint32_t volatile _size = 0;
		obj_t ** volatile _obj_ptrs = NULL;
		uint32_t volatile _head = 0;
		uint32_t volatile _tail = 0;
};

#endif
