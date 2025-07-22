typedef struct _array_circular_queue {
    //private
    int head;
    int tail;
    void ** array;
    int block_cnt;  // 재할당할 사이즈
    int array_size; // 전체 개수

} ST_CIRCUALR_QUEUE;

void cqueue_init(ST_CIRCUALR_QUEUE * cq, int block_cnt);
int cqueue_release(ST_CIRCUALR_QUEUE * cq);
int cqueue_enqueue(ST_CIRCUALR_QUEUE * cq, void * datap);
void * cqueue_dequeue(ST_CIRCUALR_QUEUE * cq);
void * cqueue_get(ST_CIRCUALR_QUEUE * cq, int index);
int cqueue_count(ST_CIRCUALR_QUEUE * cq);