#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "edit.c"
#include "recent.c"

void show_ui();
void show_logo();
void button_test();
void btn_createFile();
void btn_openFile();


int main(int argc, char *argv[])
{
    if (argc < 2){
        show_ui();
    }
}

void show_ui(){
        const char *items[] = {
        "[1] 새로운 파일 만들기",
        "[2] 기존 문서 불러오기",
        "[3] 최근 작업 목록",
        "[4] 도움말 / 사용법",
        "[5] 종료"
    };

    const int count = 5;

    int selected = 0;

    while (1) {
        system("cls");
        show_logo();
        printf("=====================================================\n");
        printf("                    NoteLite Editor\n");
        printf("=====================================================\n\n");

        for (int i = 0; i < count; i++) {
            if (i == selected)
                printf(" ▶ %s\n", items[i]);
            else
                printf("     %s\n", items[i]);
        }

        printf("\n-----------------------------------------------------\n");
        printf("[ ↑/↓ ] : 이동, [ Enter ] : 선택\n");

        int ch = getch();

        if (ch == 72) {
            if (selected > 0) selected--;
        }

        else if (ch == 80) {
            if (selected < count - 1) selected++;
        }

        else if (ch == 13) {
            switch (selected) {
                case 0: btn_createFile(); break;
                case 1: btn_openFile(); break;
                case 2: recent_menu(); break;
                case 3: button_test(); break;
                case 4: exit(0);
            }
            //return;
        }
    }

    
}
void btn_createFile()
{
    char filename[260];
    system("cls");

    printf("새로운 파일의 이름을 입력하세요 (예 : test.txt) : ");
    if (scanf("%259s", filename) != 1){
        printf ("\n 파일 생성 실패 !\n");
        getch();
        return;
    }
    //\n 제거
    int c;
    while ((c = getchar()) != '\n' && c != EOF){}

    //추후 파일 유무 검사 후 파일이 이미 있습니다, 새로 생성하시겠습니까? 추가 해야 됨.

    FILE *fp = fopen(filename, "w");
    if (fp == NULL){
        printf("\n 파일 생성 실패 !\n");
        perror("fopen");
        getch();
        return;
    }
    fclose(fp);

    recent_touch(filename);
    start_edit(filename);

}
void btn_openFile()
{
    char filename[260];
    system("cls");

    printf("불러올 파일 이름을 입력하세요 (예 : test.txt) : ");
    if (scanf("%259s", filename) != 1){
        printf("\n 파일 입력 실패 !\n");
        getch();
        return;
    }
    //\n 제거
    int c;
    while ((c = getchar()) != '\n' && c != EOF){}

    FILE *fp = fopen(filename, "r");
    if (fp == NULL){
        printf("\n 파일이 존재하지 않거나 열 수 없습니다 !\n");
        perror("fopen");
        getch();
        return;
    }
    fclose(fp);

    //최근 목록 갱신
    recent_touch(filename);

    start_edit(filename);
}



void button_test()
{
    printf("테스트입니다.");
}

void show_logo(void) {
    printf("         _   _       _        _ _ _ _\n");
    printf("        | \\ | | ___ | |_ ___ | | (_) |_ ___\n");
    printf("        |  \\| |/ _ \\\\| __/ _ \\\\| | | | __/ _\\ \n");
    printf("        | |\\  | (_) | || (_) | | | | ||  __/\n");
    printf("        |_| \\_|\\___/ \\__\\___/|_|_|_|\\__\\___|\n");
    printf("\n");
}
