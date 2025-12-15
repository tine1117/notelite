#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern char *str_dup(const char *src);

//클립보드 큐 노드
typedef struct ClipNode {
    char *text;
    struct ClipNode *next;
} ClipNode;

//클립보드 큐 상태
static ClipNode *g_front = NULL;
static ClipNode *g_rear  = NULL;
static int g_count = 0;
static int g_inited = 0;

//dequeue(앞에서 제거)
static void clip_dequeue_one(void)
{
    if (!g_front) return;

    ClipNode *n = g_front;
    g_front = n->next;
    if (!g_front) g_rear = NULL;

    free(n->text);
    free(n);
    g_count--;
    if (g_count < 0) g_count = 0;
}

void clip_init()
{
    //여러 번 호출돼도 안전하게
    if (g_inited) return;
    g_front = NULL;
    g_rear = NULL;
    g_count = 0;
    g_inited = 1;
}

int clip_is_empty()
{
    return (g_count == 0);
}

int clip_count()
{
    return g_count;
}

//enqueue 줄을 큐에 저장
int clip_copy(const char *line)
{
    if (!g_inited) clip_init();

    ClipNode *n = (ClipNode*)malloc(sizeof(ClipNode));
    if (!n) return 0;

    if (!line) line = "\n";

    n->text = str_dup(line);
    if (!n->text) {
        free(n);
        return 0;
    }
    n->next = NULL;

    if (!g_rear) {
        //첫 노드
        g_front = g_rear = n;
    } else {
        g_rear->next = n;
        g_rear = n;
    }
    g_count++;
    return 1;
}

//index번째 항목을 조회
//반환 포인터는 내부 메모리 free 하면 안됨
const char* clip_get(int index)
{
    if (index < 0 || index >= g_count) return NULL;

    ClipNode *cur = g_front;
    for (int i = 0; i < index; i++) {
        if (!cur) return NULL;
        cur = cur->next;
    }
    if (!cur) return NULL;
    return cur->text;
}

//초과하면 오래된 것부터 제거(dequeue)
void clip_enforce_limit(int max_count)
{
    if (max_count <= 0) {
        //전부 비우기
        while (g_count > 0) clip_dequeue_one();
        return;
    }

    while (g_count > max_count) {
        clip_dequeue_one();
    }
}
