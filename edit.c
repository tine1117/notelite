#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "format.c"
#include "ds.c"

void btn_edit(const char *filename);
void print_error(const char *text); //에러 처리
void btn_clipboard(Document *doc, int *cursor);  //클립보드 UI

void clip_init();
int  clip_copy(const char *line);
int  clip_count();
const char* clip_get(int index);
int  clip_is_empty();
void clip_enforce_limit(int max_count);


void start_edit(const char *filename)
{
    int running = 1;
    int menu_mode = 0;

    while (running){
        system("cls");

        FILE *fp = fopen(filename, "r");

         printf("[파일 이름 : %s ]", filename);
        if (fp == NULL) {
            printf("[파일을 읽을 수 없습니다]\n");
        } else {
            char line[512];
            while (fgets(line, sizeof(line), fp) != NULL) {
                printf("%s", line);
            }
            fclose(fp);
        }
       
        printf("\n");
        printf("---------------------------------------------------------\n");

        if (!menu_mode){
            printf ("[ESC] 메뉴 열기\n");
        }
        else { printf(" [ (Q) 에디터 종료 | (E) 수정 | (S) 편집 기록 | (ESC) 메뉴 닫기 ]\n"); }

        int ch = getch();

        if (!menu_mode){
            if (ch == 27){
                menu_mode = 1;
            }
        }else{
            if (ch == 27){
                menu_mode = 0;
            }else if (ch == 'q' || ch == 'Q') {
                running = 0;     
            } else if (ch == 'e' || ch == 'E') {
                 btn_edit(filename);
            } else if (ch == 'c' || ch == 'C') {
                
            } else if (ch == 's' || ch == 'S') {
                
            } else {
                //기타
            }
        }

    }
}

void btn_edit(const char *filename)
{
    Document doc;
    if (!document_load(&doc, filename)){
        system("cls");
        printf ("파일 불러오기 실패 : %s\n", filename);
        getch();
        return;
    }

    int cursor = 0;
    int running = 1;

    while(running)
    {
        system("cls");
        printf("[ 파일: %s", filename);
        printf(" 줄 수: %d ]\n", doc.line_count);

        if (doc.line_count == 0){
            printf("빈 파일\n");
        }else{
            int start = cursor - 10;
            int end   = cursor + 9;
            if (start < 0) start = 0;
            if (end >= doc.line_count) end = doc.line_count - 1;

            for (int i = start; i <= end; i++) {
                char mark = (i == cursor) ? '>' : ' ';
                printf("%c %3d | %s", mark, i + 1, doc.lines[i]);
                // 개행 없는 줄 안전 처리
                int len = strlen(doc.lines[i]);
                if (len == 0 || doc.lines[i][len-1] != '\n') {
                    printf("\n");
                }           
            }
        }
        printf("\n---------------------------------------------------------\n");
        printf(" [ (↑/↓) 이동 | (E) 줄 수정 | (I) 줄 삽입 | (D) 줄 삭제 ]\n");
        printf(" [ ( C) 클립보드 저장 | (V) 클립보드 | (S) 저장 후 종료 | (ESC) 저장 없이 종료 ]"); //(C) 클립보드
        printf(" Cursor(%d)\n", cursor);

        int ch = getch();

        if (ch == 0 || ch == 224) {
            int code = getch();
            if (code == 72) { // ↑
                if (cursor > 0) cursor--;
            } else if (code == 80) { // ↓
                if (cursor < doc.line_count - 1) cursor++;
            }
        }
        else if (ch == 27) { // ESC: 저장 없이 종료
            running = 0;
        }

        else if (ch == 'c' || ch == 'C') { //줄 복사(큐 저장)
            if (doc.line_count == 0) {
                print_error("복사할 줄이 없습니다.");
                continue;
            }

            //현재 줄을 큐에 저장
            if (!clip_copy(doc.lines[cursor])) {
                print_error("클립보드 저장 실패.");
            } else {
                //최대 10개 유지 (초과 시 오래된 것 제거)
                clip_enforce_limit(10);
            }
        }
        else if (ch == 'v' || ch == 'V') { //클립보드 열기
            btn_clipboard(&doc, &cursor);
        }

        else if (ch == 'i' || ch== 'I'){ //줄 삽입
            char buf[MAX_LINE_LEN];
            printf("\n새 줄 내용 입력 : \n>");

            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                print_error("입력 오류.");
                continue;
            }

            int n = strlen(buf);
            if (n == 0 || buf[n-1] != '\n') {
                if (n < MAX_LINE_LEN - 1) {
                    buf[n] = '\n';
                    buf[n+1] = '\0';
                }
            }

            int insert_pos = cursor + 1;
            if (!document_insert_line(&doc, insert_pos, buf)) {
                print_error("줄 입력 실패.");
            } else {
                cursor = insert_pos;
            }
        }
        else if (ch == 'd' || ch == 'D') { //선택 줄 삭제
            if (doc.line_count == 0) {
                print_error("삭제할 줄이 없습니다.");
                continue;
            }
            if (!document_delete_line(&doc, cursor)){
                print_error("줄 삭제 실패.");
            }else{
                if (cursor >= doc.line_count && cursor > 0){
                    cursor --;
                }
            }
        }else if (ch == 'e' || ch == 'E') {
            if (doc.line_count == 0) {
                print_error("수정할 줄이 없습니다.");
                continue;
            }

            char buf[MAX_LINE_LEN];

            printf("\n현재 줄 내용 : \n");
            printf("%s", doc.lines[cursor]);

            int len = strlen(doc.lines[cursor]);
            if (len == 0 || doc.lines[cursor][len-1] != '\n') {
                printf("\n");
            }

            printf("\n새 줄 내용 입력 (입력 없을 시 작업 취소):\n>");

            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                print_error("입력 오류");
                continue;
            }

            // 엔터만 입력 → 취소
            if (buf[0] == '\n' || buf[0] == '\0') {
                // 아무 것도 하지 않음 (작업 취소)
            } else {
                int n = strlen(buf);
                if (n == 0 || buf[n-1] != '\n') {
                    if (n < MAX_LINE_LEN - 1) {
                        buf[n] = '\n';
                        buf[n+1] = '\0';
                    }
                }

                if (!document_replace_line(&doc, cursor, buf)) {
                    print_error("작업 실패");
                }
            }
        }
    }

    document_free(&doc);
}
//클립보드(Queue) 목록을 보여주고 하나 선택해서 줄 삽입
void btn_clipboard(Document *doc, int *cursor)
{
    if (clip_is_empty()) {
        print_error("클립보드가 비어 있습니다.");
        return;
    }

    while (1) {
        system("cls");
        printf("[ 클립보드(Queue) ]\n");
        printf("---------------------------------------------------------\n");

        int count = clip_count();
        for (int i = 0; i < count; i++) {
            const char *t = clip_get(i);
            if (!t) t = "";

            //너무 길면 잘라서 보여줌
            char preview[61];
            strncpy(preview, t, 60);
            preview[60] = '\0';

            //개행 제거해서 한줄로 보기 좋게
            for (int k = 0; preview[k]; k++) {
                if (preview[k] == '\n' || preview[k] == '\r') {
                    preview[k] = '\0';
                    break;
                }
            }

            printf(" [%d] %s\n", i + 1, preview);
        }

        printf("---------------------------------------------------------\n");
        printf("번호 입력: (1~%d)  ESC: 취소\n", count);
        printf("> ");

        int ch = getch();
        if (ch == 27) { //ESC
            return;
        }

        //한 자리 숫자 선택(1~9)만 먼저 지원 (필요하면 두 자리 입력으로 확장)
        if (ch >= '1' && ch <= '9') {
            int idx = (ch - '1'); //0-based
            if (idx >= 0 && idx < count) {
                const char *text = clip_get(idx);
                if (!text) {
                    print_error("선택 오류");
                    return;
                }

                int insert_pos = (*cursor) + 1;
                if (!document_insert_line(doc, insert_pos, text)) {
                    print_error("줄 삽입 실패");
                } else {
                    *cursor = insert_pos;
                }
                return;
            }
        }
    }
}



void print_error(const char *text)
{
    printf("\n[경고] : %s\n", text);
    getch();
}