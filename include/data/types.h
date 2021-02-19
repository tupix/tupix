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

struct index_queue {
	size_t size, count;
	size_t tail, head;
	size_t* indices;
};

// NOTE(Aurel): Do not increment var when using this macro.
#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

#define INDEX_QUEUE(NAME, SIZE)                                                \
	size_t NAME[SIZE];                                                         \
	struct index_queue NAME##_q;

#define STATIC_INDEX_QUEUE(NAME, SIZE)                                         \
	static size_t NAME[SIZE];                                                  \
	static struct index_queue NAME##_q;

#define INIT_INDEX_QUEUE(NAME)                                                 \
	init_queue(&(NAME##_q), (NAME), sizeof(NAME) / sizeof(*(NAME)))

void init_queue(struct index_queue* q, size_t* indices, size_t n);
ssize_t push_index(struct index_queue* q, size_t index);
ssize_t pop_index(struct index_queue* q);

#endif /* STD_TYPES_H */
