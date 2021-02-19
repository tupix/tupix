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

/* INDEX QUEUE */
#define INDEX_QUEUE(_name, _size)                                              \
	struct _name {                                                             \
		size_t size, count;                                                    \
		size_t tail, head;                                                     \
		size_t indices[_size];                                                 \
	}
INDEX_QUEUE(index_queue, N_THREADS);

// NOTE(Aurel): Do not increment var when using this macro.
#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

void init_queue(struct index_queue* q);
ssize_t push_index(struct index_queue* q, size_t index);
ssize_t pop_index(struct index_queue* q);

#endif /* STD_TYPES_H */
