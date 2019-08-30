#include "mapping.h"

int max_q_length;

void enqueuePair(Addr_Pair pair)
{
	if (queue.length >= max_q_length)
	{
		printf("Not enough room in pair queue\n");
		exit(EXIT_FAILURE);
	}
	queue.pairs[queue.back].tree_address = pair.tree_address;
	queue.pairs[queue.back].heap_address = pair.heap_address;
	queue.pairs[queue.back].chunk = pair.chunk;
	queue.length++;

	if (queue.back < max_q_length - 1)
	{
		queue.back++;
	}
	else
	{
		queue.back = 0;
	}
}
void enqueueObject(Object obj)
{
	if (header_queue.length >= MAX_Q_LENGTH_GROUP)
	{
		printf("Not enough room in header queue\n");
		exit(EXIT_FAILURE);
	}
	header_queue.objects[header_queue.back].obj_header_address = obj.obj_header_address;
	strcpy_s(header_queue.objects[header_queue.back].name, NAME_LENGTH, obj.name);
	header_queue.objects[header_queue.back].this_tree_address = obj.this_tree_address;
	header_queue.objects[header_queue.back].prev_tree_address = obj.prev_tree_address;
	header_queue.length++;

	if (header_queue.back < MAX_Q_LENGTH_GROUP - 1)
	{
		header_queue.back++;
	}
	else
	{
		header_queue.back = 0;
	}
}
void priorityEnqueuePair(Addr_Pair pair)
{
	if (queue.length >= max_q_length)
	{
		printf("Trying to priority enqueue: Not enough room in pair queue\n");
		exit(EXIT_FAILURE);
	}
	if (queue.front - 1 < 0)
	{
		queue.pairs[max_q_length - 1].tree_address = pair.tree_address;
		queue.pairs[max_q_length - 1].heap_address = pair.heap_address;
		queue.front = max_q_length - 1;
	}
	else
	{
		queue.pairs[queue.front - 1].tree_address = pair.tree_address;
		queue.pairs[queue.front - 1].heap_address = pair.heap_address;
		queue.front--;
	}
	queue.length++;
}
void priorityEnqueueObject(Object obj)
{
	if (header_queue.length >= MAX_Q_LENGTH_GROUP)
	{
		printf("Trying to priority enqueue: Not enough room in header queue\n");
		exit(EXIT_FAILURE);
	}
	if (header_queue.front - 1 < 0)
	{
		header_queue.objects[MAX_Q_LENGTH_GROUP - 1].obj_header_address = obj.obj_header_address;
		strcpy_s(header_queue.objects[MAX_Q_LENGTH_GROUP - 1].name, NAME_LENGTH, obj.name);
		header_queue.objects[MAX_Q_LENGTH_GROUP - 1].this_tree_address = obj.this_tree_address;
	header_queue.objects[MAX_Q_LENGTH_GROUP - 1].prev_tree_address = obj.prev_tree_address;
		header_queue.front = MAX_Q_LENGTH_GROUP - 1;
	}
	else
	{
		header_queue.objects[header_queue.front - 1].obj_header_address = obj.obj_header_address;
		strcpy_s(header_queue.objects[header_queue.front - 1].name, NAME_LENGTH, obj.name);
		header_queue.objects[header_queue.front].this_tree_address = obj.this_tree_address;
	header_queue.objects[header_queue.front].prev_tree_address = obj.prev_tree_address;
		header_queue.front--;
	}
	header_queue.length++;
}
Addr_Pair dequeuePair()
{
	Addr_Pair pair;
	pair.tree_address = queue.pairs[queue.front].tree_address;
	pair.heap_address = queue.pairs[queue.front].heap_address;
	pair.chunk = queue.pairs[queue.front].chunk;
	if (queue.front + 1 < max_q_length)
	{
		queue.front++;
	}
	else
	{
		queue.front = 0;
	}
	queue.length--;
	return pair;
}
Object dequeueObject()
{
	Object obj;
	obj.obj_header_address = header_queue.objects[header_queue.front].obj_header_address;
	strcpy_s(obj.name, NAME_LENGTH, header_queue.objects[header_queue.front].name);
	obj.prev_tree_address = header_queue.objects[header_queue.front].prev_tree_address;
	obj.this_tree_address = header_queue.objects[header_queue.front].this_tree_address;
	if (header_queue.front + 1 < MAX_Q_LENGTH_GROUP)
	{
		header_queue.front++;
	}
	else
	{
		header_queue.front = 0;
	}
	header_queue.length--;
	return obj;
}
void flushQueue(QueueType type, int free_queue)
{
	if (free_queue)
	{
		free(queue.pairs);
	}
	queue.length = 0;
	queue.front = 0;
	queue.back = 0;
	if (type == GROUP_T)
	{
		queue.pairs = malloc(MAX_Q_LENGTH_GROUP*sizeof(Addr_Pair));
		max_q_length = MAX_Q_LENGTH_GROUP;
	}
	else if (type == CHUNK_T)
	{
		queue.pairs = malloc(MAX_Q_LENGTH_CHUNK*sizeof(Addr_Pair));
		max_q_length = MAX_Q_LENGTH_CHUNK;
	}
}
void flushHeaderQueue()
{
	header_queue.length = 0;
	header_queue.front = 0;
	header_queue.back = 0;
}
