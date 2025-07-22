
// #include "circular_queue.h"
#include "common.h"

#define ARRAY_IDX(idx, array_size) ((idx) % (array_size))



static int cqueue_resize(ST_CIRCUALR_QUEUE * cq, int new_array_size)
{
    // printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(void *) * new_array_size);
    void ** new_array = malloc(sizeof(void *) * new_array_size);
    if (new_array == NULL)
        return -1;

    if (cq->array)
    {
        int i;
        int newidx = 0;
        for (i = cq->head; i < cq->tail; i++, newidx++)
        {
            new_array[ARRAY_IDX(newidx, new_array_size)] = cq->array[ARRAY_IDX(i, cq->array_size)];
        }

        cq->head = 0;
        cq->tail = newidx;
        __free(cq->array);
    }

    cq->array = new_array;
    cq->array_size = new_array_size;

    return 0;
}

void cqueue_init(ST_CIRCUALR_QUEUE * cq, int block_cnt)
{
    cq->array = NULL;
    cq->head = 0;
    cq->tail = 0;

    cq->block_cnt = block_cnt;
    cq->array_size = 0;
}

int cqueue_release(ST_CIRCUALR_QUEUE * cq)
{
    if (cq->array)
    {
        __free(cq->array);
    }

    return 0;
}

int cqueue_enqueue(ST_CIRCUALR_QUEUE * cq, void * datap)
{
    if (cq->array == NULL)
    {
        if (cqueue_resize(cq, cq->block_cnt) < 0)
        {
            return -1;
        }
    }

    if (ARRAY_IDX(cq->tail + 1, cq->array_size) == ARRAY_IDX(cq->head, cq->array_size))
    {
        if (cqueue_resize(cq, cq->array_size + cq->block_cnt) < 0)
        {
            return -1;
        }
    }

    cq->array[ARRAY_IDX(cq->tail, cq->array_size)] = datap;
    cq->tail++;

    return 0;
}

void * cqueue_dequeue(ST_CIRCUALR_QUEUE * cq)
{
    if (cq->head == cq->tail)
        return NULL;

    void * p = cq->array[ARRAY_IDX(cq->head, cq->array_size)];
    cq->head++;
    return p;
}

// index의 값을 읽기만 한다.
void * cqueue_get(ST_CIRCUALR_QUEUE * cq, int index)
{

    if (cqueue_count(cq) > index && index >= 0)
    {
        return cq->array[ARRAY_IDX(cq->head + index, cq->array_size)];
    }

    return NULL;
}

int cqueue_count(ST_CIRCUALR_QUEUE * cq)
{
    return cq->tail - cq->head;
}