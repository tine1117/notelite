#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

typedef struct RecentNode {
    char filename[260];
    int last_open_seq;     //프로그램 실행 중 증가하는 시퀀스
    int open_count;        //열린 횟수
    struct RecentNode *left;
    struct RecentNode *right;
} RecentNode;

static RecentNode *g_recent_root = NULL;
static int g_open_seq = 0;

//---------------------------------------------------------
// 내부 유틸

static RecentNode* recent_new(const char *filename)
{
    RecentNode *n = (RecentNode*)malloc(sizeof(RecentNode));
    if (!n) return NULL;

    strncpy(n->filename, filename, sizeof(n->filename) - 1);
    n->filename[sizeof(n->filename) - 1] = '\0';

    n->last_open_seq = 0;
    n->open_count = 0;
    n->left = NULL;
    n->right = NULL;
    return n;
}

static int recent_cmp_filename(const char *a, const char *b)
{
    //알파벳(사전) 순 정렬 기준
    return strcmp(a, b);
}

static RecentNode* recent_find(RecentNode *root, const char *filename)
{
    if (!root) return NULL;

    int c = recent_cmp_filename(filename, root->filename);
    if (c == 0) return root;
    if (c < 0) return recent_find(root->left, filename);
    return recent_find(root->right, filename);
}

static RecentNode* recent_insert(RecentNode *root, RecentNode *node)
{
    if (!root) return node;

    int c = recent_cmp_filename(node->filename, root->filename);
    if (c < 0) root->left = recent_insert(root->left, node);
    else if (c > 0) root->right = recent_insert(root->right, node);
    else {
        //동일 filename이면 기존 노드 유지(호출부에서 find 후 갱신)
        free(node);
    }
    return root;
}

static void recent_inorder_print(RecentNode *root)
{
    if (!root) return;
    recent_inorder_print(root->left);
    printf(" - %s (open:%d, seq:%d)\n", root->filename, root->open_count, root->last_open_seq);
    recent_inorder_print(root->right);
}

static int recent_count_nodes(RecentNode *root)
{
    if (!root) return 0;
    return 1 + recent_count_nodes(root->left) + recent_count_nodes(root->right);
}

static void recent_collect_inorder(RecentNode *root, RecentNode **arr, int *idx)
{
    if (!root) return;
    recent_collect_inorder(root->left, arr, idx);
    arr[*idx] = root;
    (*idx)++;
    recent_collect_inorder(root->right, arr, idx);
}

//---------------------------------------------------------
// 최근순 정렬 (삽입 정렬) : last_open_seq 내림차순

static void recent_sort_by_seq(RecentNode **arr, int n)
{
    for (int i = 1; i < n; i++) {
        RecentNode *key = arr[i];
        int j = i - 1;

        //내림차순: seq 큰 것이 앞으로
        while (j >= 0 && arr[j]->last_open_seq < key->last_open_seq) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

//---------------------------------------------------------

void recent_touch(const char *filename)
{
    if (!filename || filename[0] == '\0') return;

    //시퀀스 증가
    g_open_seq++;

    RecentNode *n = recent_find(g_recent_root, filename);
    if (!n) {
        //없으면 새로 만들어 BST에 삽입
        RecentNode *new_node = recent_new(filename);
        if (!new_node) return;

        new_node->open_count = 1;
        new_node->last_open_seq = g_open_seq;

        g_recent_root = recent_insert(g_recent_root, new_node);
        return;
    }

    //있으면 갱신
    n->open_count++;
    n->last_open_seq = g_open_seq;
}

//최근 작업 목록 UI
void recent_menu()
{
    while (1) {
        system("cls");
        printf("[ 최근 작업 목록 ]\n");
        printf("---------------------------------------------------------\n");
        printf(" [A] 알파벳 순 출력(BST inorder)\n");
        printf(" [R] 최근순 출력(정렬 알고리즘)\n");
        printf(" [ESC] 뒤로가기\n");
        printf("---------------------------------------------------------\n");

        int ch = getch();
        if (ch == 27) return;

        if (ch == 'a' || ch == 'A') {
            system("cls");
            printf("[ 알파벳 순 출력 (BST inorder) ]\n");
            printf("---------------------------------------------------------\n");
            if (!g_recent_root) {
                printf("최근 목록이 비어 있습니다.\n");
            } else {
                recent_inorder_print(g_recent_root);
            }
            printf("---------------------------------------------------------\n");
            printf("아무 키나 누르면 돌아갑니다.\n");
            getch();
        }
        else if (ch == 'r' || ch == 'R') {
            system("cls");
            printf("[ 최근순 출력 (삽입 정렬: last_open_seq) ]\n");
            printf("---------------------------------------------------------\n");

            int count = recent_count_nodes(g_recent_root);
            if (count <= 0) {
                printf("최근 목록이 비어 있습니다.\n");
            } else {
                RecentNode **arr = (RecentNode**)malloc(sizeof(RecentNode*) * count);
                if (!arr) {
                    printf("메모리 할당 실패\n");
                } else {
                    int idx = 0;
                    recent_collect_inorder(g_recent_root, arr, &idx);

                    //정렬 알고리즘으로 최근순 정렬
                    recent_sort_by_seq(arr, count);

                    for (int i = 0; i < count; i++) {
                        printf(" %2d) %s (open:%d, seq:%d)\n",
                               i + 1, arr[i]->filename, arr[i]->open_count, arr[i]->last_open_seq);
                    }
                    free(arr);
                }
            }

            printf("---------------------------------------------------------\n");
            printf("아무 키나 누르면 돌아갑니다.\n");
            getch();
        }
    }
}