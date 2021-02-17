#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <config.h>

typedef enum bool { true = 1, false = 0 } bool;

/* NUMERICS */
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef uint32 size_t;
typedef int32 ssize_t;

#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

/**
 * INDEX QUEUE
 * NOTE(Aurel): This macro creates a new index queue and its functions. Be aware
 * that you should only ever use this inside the .c-files as this might conflict
 * with other index_queues in other .c-files otherwise.
 * When using this function it creates a new struct with the given name and
 * functions to initialize the index queue, push an index onto the queue and pop
 * an index from the queue.
 */
#define INDEX_QUEUE(_name, _size)                                              \
	struct _name {                                                             \
		size_t size, count;                                                    \
		size_t tail, head;                                                     \
		size_t indices[_size];                                                 \
	};                                                                         \
                                                                               \
	void init_queue(struct _name* q)                                           \
	{                                                                          \
		memset(q, 0, sizeof(*q));                                              \
		q->size = _size;                                                       \
	}                                                                          \
                                                                               \
	ssize_t push_index(struct _name* q, size_t index)                          \
	{                                                                          \
		if (!q) {                                                              \
			klog(WARNING, "Invalid index_queue (NULL)");                       \
			return -1;                                                         \
		} else if (index <= 0 || index > q->size) {                            \
			klog(WARNING, "Index %i out of bounds", index);                    \
			return -1;                                                         \
		} else if (q->count >= q->size) {                                      \
			return 0;                                                          \
		}                                                                      \
                                                                               \
		q->indices[q->tail] = index;                                           \
		circle_forward(q->tail, q->size);                                      \
		++(q->count);                                                          \
		return index;                                                          \
	}                                                                          \
                                                                               \
	ssize_t pop_index(struct _name* q)                                         \
	{                                                                          \
		if (!q) {                                                              \
			klog(WARNING, "Invalid index_queue (NULL)");                       \
			return -1;                                                         \
		} else if (!q->count) {                                                \
			return 0;                                                          \
		}                                                                      \
                                                                               \
		size_t index = q->indices[q->head];                                    \
		circle_forward(q->head, q->size);                                      \
		--(q->count);                                                          \
		return index;                                                          \
	}

#endif /* STD_TYPES_H */
