#include "message_queue.h"

template<class obj_t>
MessagePtrQueue<obj_t>::MessagePtrQueue() {
}

template<class obj_t>
MessagePtrQueue<obj_t>::MessagePtrQueue(uint32_t size) {
	this->init(size);
}

template<class obj_t>
void MessagePtrQueue<obj_t>::init(uint32_t size) {
	this->_size = size;

	this->_obj_ptrs = new obj_t*[size];
	INVARIANT(this->_obj_ptrs != NULL);
	for (size_t i = 0; i < size; i++) {
		this->_obj_ptrs[i] = NULL;
	}
}

template<class obj_t>
bool MessagePtrQueue<obj_t>::write(obj_t *newobj) {
	INVARIANT(newobj != NULL);
	if (((this->_head + 1) % this->_size) != this->_tail) {
		INVARIANT(this->_obj_ptrs[this->_head] == NULL);
		this->_obj_ptrs[this->_head] = newobj;
		this->_head = (this->_head + 1) % this->_size;
		return true;
	}
	return false;
}

template<class obj_t>
obj_t * MessagePtrQueue<obj_t>::read() {
	obj_t *result = NULL;
	if (this->_tail != this->_head) {
		result = this->_obj_ptrs[this->_tail];
		this->_obj_ptrs[this->_tail] = NULL;
		this->tail = (this->_tail + 1) % this->_size;
		INVARIANT(result != NULL);
	}
	return result;
}

template<class obj_t>
MessagePtrQueue<obj_t>::~MessagePtrQueue() {
	if (this->_obj_ptrs != NULL) {
		for (size_t i = 0; i < this->_size; i++) {
			if (this->_obj_ptrs[i] != NULL) {
				delete this->_obj_ptrs[i];
				this->_obj_ptrs[i] = NULL;
			}
		}
		delete [] this->_obj_ptrs;
		this->_obj_ptrs = NULL;
	}
}
