#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#define MAX_LINE_LEN 100

typedef struct {
    char **lines; //문자열 배열 관리
    int line_count; //사용중인 줄
    int capacity; //lins 배열 최대 슬롯 개수
    /*
        lines[line_count-1] = 마지막 줄
    */
} Document;

//------------------------------------------------------------------
/*
typedef struct BstNode {
    int key;                //줄 번호(정렬 기준)
    const char *text;       //Document의 lines[i] 포인터를 그대로 참조
    struct BstNode *left;
    struct BstNode *right;
} BstNode;

static BstNode* bst_new(int key, const char *text)
{
    BstNode *n = (BstNode*)malloc(sizeof(BstNode));
    if (!n) return NULL;

    n->key = key;
    n->text = text;
    n->left = NULL;
    n->right = NULL;
    return n;
}

static BstNode* bst_insert(BstNode *root, int key, const char *text)
{
    if (!root) return bst_new(key, text);

    if (key < root->key) {
        root->left = bst_insert(root->left, key, text);
    } else if (key > root->key) {
        root->right = bst_insert(root->right, key, text);
    } else {
        //같은 key는 덮어쓰기(사실상 발생하지 않음)
        root->text = text;
    }
    return root;
}

static void bst_inorder_write(BstNode *root, FILE *fp)
{
    if (!root) return;

    bst_inorder_write(root->left, fp);

    if (root->text) {
        fputs(root->text, fp);

        //개행 없는 줄 안전 처리
        int len = (int)strlen(root->text);
        if (len == 0 || root->text[len-1] != '\n') {
            fputc('\n', fp);
        }
    }

    bst_inorder_write(root->right, fp);
}

static void bst_free(BstNode *root)
{
    if (!root) return;
    bst_free(root->left);
    bst_free(root->right);
    free(root);
}*/


char *str_dup(const char *src){ //기존 strdup, 문자열 길이 계산, 할당 후 버퍼 포인터 반환
    size_t len = strlen(src);
    char *dst = (char *)malloc(len + 1);
    if (!dst) return NULL;
    memcpy(dst, src, len + 1);
    return dst;
}

int document_init(Document *doc){ //문서 초기화
    doc -> capacity = 60;
    doc -> line_count = 0;
    doc -> lines = (char **)malloc(sizeof(char *) * doc->capacity);
    if (!doc->lines) return 0;
    return 1;
}

void document_free(Document *doc){ //Document 메모리 초기화
    if(!doc -> lines) return;
    for (int i = 0; i < doc->line_count; i++){
        free(doc->lines[i]);
    }
    free(doc->lines);
    doc->lines = NULL;
    doc->line_count = 0;
    doc->capacity = 0;
}

int docuemnt_add_capacity(Document *doc, int needed){
    //텍스트 작성 공간 확대 부분
    if (needed <= doc->capacity) return 1;
    int new_cap = doc->capacity * 2;
    if (new_cap < needed) new_cap = needed;
    char **new_lines = (char **)realloc(doc->lines, sizeof(char *) * new_cap);
    if (!new_lines) return 0;
    doc->lines = new_lines;
    doc->capacity = new_cap;
    return 1;
}

int document_load(Document *doc, const char *filename){ //파일 읽기
    if (!document_init(doc)) return 0; //문서 공간 초기화
    FILE *fp = fopen(filename, "r");
    if (!fp) { return 1; }

    char buf[MAX_LINE_LEN];
    while(fgets(buf, sizeof(buf), fp) != NULL){ //fgets으로 한줄 씩 읽음

        if (!docuemnt_add_capacity(doc, doc->line_count + 1)) {  //공간이 부족시 공간 추가
            fclose(fp);
            return 0;
        }

        doc->lines[doc->line_count] = str_dup(buf); //복사
        if (!doc->lines[doc->line_count]) {
            fclose(fp);
            return 0;
        }
        doc->line_count++; //줄 증가
    }
    fclose(fp);
    return 1;
}

int document_delete_line(Document *doc, int index) {
    if (doc->line_count == 0) return 0;
    if (index < 0 || index >= doc->line_count) return 0;

    free(doc->lines[index]);

    for (int i = index; i < doc->line_count - 1; i++) {
        doc->lines[i] = doc->lines[i + 1];
    }
    doc->line_count--;

    return 1;
}

int document_insert_line(Document *doc, int index, const char *text) {
    if (index < 0) index = 0;
    if (index > doc->line_count) index = doc->line_count;

    if (!docuemnt_add_capacity(doc, doc->line_count + 1)) return 0;

    // 뒤에서부터 한 칸씩 밀어냄
    for (int i = doc->line_count; i > index; i--) {
        doc->lines[i] = doc->lines[i - 1];
    }

    doc->lines[index] = str_dup(text);
    if (!doc->lines[index]) return 0;

    doc->line_count++;
    return 1;
}

int document_replace_line(Document *doc, int index, const char *text) {
    if (index < 0 || index >= doc->line_count) return 0;

    free(doc->lines[index]);
    doc->lines[index] = str_dup(text); //버퍼 내용을 새 메모리에 복사해서 저장 buf = 임시 버퍼
    if (!doc->lines[index]) return 0;

    return 1;
}

int document_save(Document *doc, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;

    for (int i = 0; i < doc->line_count; i++) {
        fputs(doc->lines[i], fp);
        // 필요하면 개행 없을 때 '\n' 추가 로직 넣어도 됨
    }

    fclose(fp);
    return 1;
}

/*
int document_save(Document *doc, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;

    //Tree(BST)로 줄을 저장하고 inorder로 출력
    BstNode *root = NULL;

    for (int i = 0; i < doc->line_count; i++) {
        root = bst_insert(root, i, doc->lines[i]); //key = 줄 번호
    }

    bst_inorder_write(root, fp);
    bst_free(root);

    fclose(fp);
    return 1;
}
/**/



