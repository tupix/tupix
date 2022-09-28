#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <config.h>

typedef enum bool { false = 0, true = 1 } bool;

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

#define MARK_ALL_FREE(Q)                                                       \
	for (size_t i = 0; i < (Q).size; ++i) {                                    \
		push_index(&(Q), i + 1);                                               \
	}

/* TODO:
	size_t NAME[SIZE];
	struct index_queue NAME##_q = { .size = SIZE, .indices = NAME };
*/
#define INDEX_QUEUE(NAME, SIZE)                                                \
	size_t NAME[SIZE];                                                         \
	struct index_queue NAME##_q;

#define STATIC_INDEX_QUEUE(NAME, SIZE)                                         \
	static size_t NAME[SIZE];                                                  \
	static struct index_queue NAME##_q;

#define INIT_INDEX_QUEUE(NAME)                                                 \
	init_queue(&(NAME##_q), (NAME), sizeof(NAME) / sizeof(*(NAME)))

/*
 * Null the struct and the buffer and set buffer and size
 */
void init_queue(struct index_queue* q, size_t* indices, size_t n);

/*
 * Push index to index_queue.
 * Allows only 1-based indices until (including) q->size.
 *
 * @return 0 on any fatal error.
 */
ssize_t push_index(struct index_queue* q, size_t index);

/*
 * Pop index from index_queue.
 *
 * @return -1 on any fatal error.
 */
ssize_t pop_index(struct index_queue* q);

#endif /* STD_TYPES_H */
