#include <data/types.h>
#include <std/log.h>
#include <std/mem.h>

/*
 * Null the struct but set the size
 */
void
init_queue(struct index_queue* q)
{
	kmemset(q, 0, sizeof(*q));
	q->size = sizeof(q->indices) / sizeof(*(q->indices));
}

// NOTE(Aurel): Do not increment var when using this macro.
#define circle_forward(var, size) (var) = (var) + 1 >= (size) ? 0 : (var) + 1

/*
 * Push index to index_queue.
 *
 * @return 0 if queue is full and -1 on any other fatal error.
 */
ssize_t
push_index(struct index_queue* q, size_t index)
{
	if (!q) {
		klog(WARNING, "Invalid index_queue (NULL)");
		return -1;
	} else if (index <= 0 || index > q->size) {
		klog(WARNING, "Index %i out of bounds", index);
		return -1;
	} else if (q->count >= q->size) {
		return 0;
	}

	q->indices[q->tail] = index;
	circle_forward(q->tail, q->size);
	++(q->count);
	return index;
}

/*
 * Pop index from index_queue.
 *
 * @return 0 if queue is empty and -1 on any other fatal error.
 */
ssize_t
pop_index(struct index_queue* q)
{
	if (!q) {
		klog(WARNING, "Invalid index_queue (NULL)");
		return -1;
	} else if (!q->count) {
		return 0;
	}

	size_t index = q->indices[q->head];
	circle_forward(q->head, q->size);
	--(q->count);
	return index;
}
