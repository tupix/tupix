#include <data/types.h>
#include <std/log.h>
#include <std/mem.h>

// TODO: flag if memset or not
void
init_queue(struct index_queue* q, size_t* indices, size_t n)
{
	memset(q, 0, sizeof(*q));
	memset(indices, 0, n * sizeof(*indices));
	q->indices = indices;
	q->size    = n;
}

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
		klog(WARNING, "Queue is full.");
		return -1;
	}

	q->indices[q->tail] = index;
	circle_forward(q->tail, q->size);
	++(q->count);
	return index;
}

ssize_t
pop_index(struct index_queue* q)
{
	if (!q) {
		klog(WARNING, "Invalid index_queue (NULL)");
		return -1;
	} else if (!q->count) {
		klog(WARNING, "Queue is empty.");
		return -1;
	}

	size_t index = q->indices[q->head];
	circle_forward(q->head, q->size);
	--(q->count);
	return index;
}
