#define _CRT_SECURE_NO_WARNINGS
#undef UNICODE
#undef _UNICODE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <easyx.h>
#include <graphics.h>
#include <mmsystem.h>
#include <Windows.h>

#pragma comment(lib, "winmm.lib")

#define ROW 16
#define COL 30

IMAGE imgs[12];

enum MenuOptions {
    MENU_START_GAME,
    MENU_SETTINGS,
    MENU_EXIT
};

// 按钮结构体统一定义
struct button {
    int x, y, w, h;
    char* pImageFileUp; // 正常状态图片文件路径
    char* pImageFileDown; // 按下状态图片文件路径
    int isDown; // 按钮状态，0 表示正常状态，1 表示按下状态
};

// 创建按钮函数
struct button* createButton(int x, int y, int w, int h, const char* pImageFileUp, const char* pImageFileDown) {
    struct button* p = (struct button*)malloc(sizeof(struct button));
    p->x = x;
    p->y = y;
    p->w = w;
    p->h = h;
    p->pImageFileUp = (char*)malloc(strlen(pImageFileUp) + 1);
    strcpy(p->pImageFileUp, pImageFileUp);
    p->pImageFileDown = (char*)malloc(strlen(pImageFileDown) + 1);
    strcpy(p->pImageFileDown, pImageFileDown);
    p->isDown = 0; // 默认是正常状态
    return p;
}

// 绘制按钮函数
void draw_button(struct button* p) {
    IMAGE img;
    if (p->isDown) {
        loadimage(&img, p->pImageFileDown, p->w, p->h); // 加载按下状态图片
    }
    else {
        loadimage(&img, p->pImageFileUp, p->w, p->h); // 加载正常状态图片
    }
    putimage(p->x, p->y, &img); // 绘制按钮图片
}

// 检查点是否在矩形内
int point_in_rect(int px, int py, struct button* btn) {
    return px >= btn->x && px <= btn->x + btn->w &&
        py >= btn->y && py <= btn->y + btn->h;
}

// 切换音乐函数
void switch_music(const char** musicFiles, int* currentMusicIndex, int musicCount) {
    mciSendString("close bgm", NULL, 0, NULL); // 关闭当前音乐
    (*currentMusicIndex)++;
    if (*currentMusicIndex >= musicCount) {
        *currentMusicIndex = 0; // 循环回到第一个音乐文件
    }
    char command[256];
    sprintf(command, "open \"%s\" alias bgm", musicFiles[*currentMusicIndex]);
    mciSendString(command, NULL, 0, NULL);
    mciSendString("play bgm repeat", NULL, 0, NULL);
}

// 资源加载
void loadResources() {
    for (int i = 0; i < 12; i++) {
        char imgPath[50] = { 0 };
        sprintf_s(imgPath, "./images/%d.jpg", i);
        loadimage(&imgs[i], imgPath, 50, 50);
    }
}

// 扫雷游戏相关函数声明
void init(int map[][COL], int rows, int cols, int mine_num);
void draw(int map[][COL], int rows, int cols);
void mouseMsg(ExMessage* msg, int map[][COL], int rows, int cols);
void boomspace(int map[][COL], int row, int col, int rows, int cols);
int judge(int map[][COL], int row, int col, int rows, int cols, int mine_num);

// 菜单绘制
void draw_menu() {
    IMAGE menu;
    loadimage(&menu, "./images/menu.jpg", 500, 500);
    putimage(0, 0, &menu);
}

// 主函数
int main() {
    aa:initgraph(500, 500, EW_SHOWCONSOLE);
    srand((unsigned)time(NULL));

    // 背景音乐文件列表
    const char* musicFiles[] = {
        "./images/background.mp3",
        "./images/background1.mp3",
        "./images/background2.mp3",
        // 可以在这里添加更多的音乐文件
    };
    int musicCount = sizeof(musicFiles) / sizeof(musicFiles[0]);
    int currentMusicIndex = -1;

    // 打开并播放初始音乐
    switch_music(musicFiles, &currentMusicIndex, musicCount);
    
    IMAGE menu;
    loadimage(&menu, "./images/menu.jpg", 500, 500);
    putimage(0, 0, &menu);
    // 创建带有图片的按钮
    struct button* easy = createButton(55, 248, 161, 43, "./images/easyup.jpg", "./images/easydown.jpg");
    struct button* normal = createButton(55, 303, 161, 43, "./images/normalup.jpg", "./images/normaldown.jpg");
    struct button* hard = createButton(55, 358, 161, 43, "./images/hardup.jpg", "./images/harddown.jpg");
    struct button* out = createButton(55, 413, 161, 43, "./images/outup.jpg", "./images/outdown.jpg");
    struct button* bgm = createButton(233, 413, 230, 43, "./images/bgmup.jpg", "./images/bgmdown.jpg");
    struct button* next1 = createButton(53, 228, 83,48, "./images/next1up.jpg", "./images/next1down.jpg");
    struct button* out1 = createButton(320, 228, 83,48, "./images/out1up.jpg", "./images/out1down.jpg");
    struct button* next2 = createButton(95, 406, 146,84, "./images/next2up.jpg", "./images/next2down.jpg");
    struct button* out2 = createButton(570, 406, 146, 84, "./images/out2up.jpg", "./images/out2down.jpg");
    // 绘制所有按钮
    draw_menu();
    draw_button(easy);
    draw_button(normal);
    draw_button(hard);
    draw_button(out);
    draw_button(bgm);

    // 主循环
    while (true) {
        MOUSEMSG m = GetMouseMsg();
        switch (m.uMsg) {
        case WM_LBUTTONDOWN:
            // 检查鼠标点击是否在按钮范围内
            if (point_in_rect(m.x, m.y, easy)) {
                easy->isDown = 1;
                draw_button(easy);
                Sleep(200);
                if (easy->isDown) {
                    easy->isDown = 0;
                    draw_button(easy);
                }
                Sleep(200);
                closegraph(); // 关闭菜单界面
                initgraph(9* 50,9* 50, EW_SHOWCONSOLE);
                int map[ROW][COL] = { 0 };
                init(map, 9,9,10);
                draw(map,9,9);
                // 游戏主循环
                while (true) {
                    ExMessage msg;
                    while (peekmessage(&msg, EM_MOUSE)) {
                        if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN) {
                            mouseMsg(&msg, map,9,9);
                            int ret = judge(map, msg.y / 50, msg.x / 50,9,9,10);
                            if (ret == -1 && msg.message != WM_RBUTTONDOWN) {
                                PlaySound("./images/boom.wav", NULL, SND_ASYNC | SND_FILENAME);
                                draw(map,9,9);
                                int select = MessageBox(GetHWnd(), "noobs!", "you lose!", MB_OKCANCEL);
                                if (select == IDOK) {
                                    goto aa;
                                }
                                else {
                                    closegraph();
                                    return 0;
                                }
                            }
                            else if (ret == 1) {
                                draw(map, 9, 9);
                                IMAGE win;
                                loadimage(&win, "./images/win1.jpg", 450, 450);
                                putimage(0, 0, &win);
                                draw_button(next1);
                                draw_button(out1);
                                mciSendString("stop bgm", NULL, 0, NULL);
                                PlaySound("./images/win.wav", NULL, SND_ASYNC | SND_FILENAME);
                                Sleep(4000);
                                while (true) {
                                    MOUSEMSG m = GetMouseMsg();
                                    if (m.uMsg == WM_LBUTTONDOWN) {
                                        if (point_in_rect(m.x, m.y, next1)) {
                                            next1->isDown = 1;
                                            draw_button(next1);
                                            Sleep(200);
                                            if (next1->isDown) {
                                                next1->isDown = 0;
                                                draw_button(next1);
                                            }
                                            Sleep(200);
                                            closegraph(); // 关闭当前游戏界面
                                            goto aa; // 返回菜单页面
                                        }
                                        else if (point_in_rect(m.x, m.y, out1)) {
                                            out1->isDown = 1;
                                            draw_button(out1);
                                            Sleep(200);
                                            if (out1->isDown) {
                                                out1->isDown = 0;
                                                draw_button(out1);
                                            }
                                            Sleep(200);
                                            closegraph(); // 关闭当前游戏界面
                                            return 0; // 退出程序
                                        }
                                    }
                                }
                            }
                        }
                    }
                    draw(map,9,9);
                }
                break;
            }
            else if (point_in_rect(m.x, m.y, normal)) {
                normal->isDown = 1;
                draw_button(normal);
                Sleep(200);
                if (normal->isDown) {
                    normal->isDown = 0;
                    draw_button(normal);
                }
                Sleep(200);
                closegraph(); // 关闭菜单界面
                initgraph(16* 50,16*50, EW_SHOWCONSOLE);
                int map[ROW][COL] = { 0 };
                init(map,16,16,40);
                draw(map,16,16);
                while (true) {
                    ExMessage msg;
                    while (peekmessage(&msg, EM_MOUSE)) {
                        if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN) {
                            mouseMsg(&msg, map,16,16);
                            int ret = judge(map, msg.y / 50, msg.x / 50,16,16,40);
                            if (ret == -1 && msg.message != WM_RBUTTONDOWN) {
                                PlaySound("./images/boom.wav", NULL, SND_ASYNC | SND_FILENAME);
                                draw(map,16,16);
                                int select = MessageBox(GetHWnd(), "noobs!", "you lose!", MB_OKCANCEL);
                                if (select == IDOK) {
                                    goto aa;
                                }
                                else {
                                    closegraph();
                                    return 0;
                                }
                            }
                            else if (ret == 1) {
                                draw(map,16,16);
                                IMAGE win;
                                loadimage(&win, "./images/win2.jpg",800,800);
                                putimage(0, 0, &win);
                                draw_button(next2);
                                draw_button(out2);
                                mciSendString("stop bgm", NULL, 0, NULL);
                                PlaySound("./images/win.wav", NULL, SND_ASYNC | SND_FILENAME);
                                Sleep(4000);
                                while (true) {
                                    MOUSEMSG m = GetMouseMsg();
                                    if (m.uMsg == WM_LBUTTONDOWN) {
                                        if (point_in_rect(m.x, m.y, next2)) {
                                            next2->isDown = 1;
                                            draw_button(next2);
                                            Sleep(200);
                                            if (next2->isDown) {
                                                next2->isDown = 0;
                                                draw_button(next2);
                                            }
                                            Sleep(200);
                                            closegraph(); // 关闭当前游戏界面
                                            goto aa; // 返回菜单页面
                                        }
                                        else if (point_in_rect(m.x, m.y, out2)) {
                                            out2->isDown = 1;
                                            draw_button(out2);
                                            Sleep(200);
                                            if (out2->isDown) {
                                                out2->isDown = 0;
                                                draw_button(out2);
                                            }
                                            Sleep(200);
                                            closegraph(); // 关闭当前游戏界面
                                            return 0; // 退出程序
                                        }
                                    }
                                }
                            }
                        }
                    }
                    draw(map,16,16);
                }
                break;
            }
            else if (point_in_rect(m.x, m.y, hard)) {
                hard->isDown = 1;
                draw_button(hard);
                Sleep(200);
                if (hard->isDown) {
                    hard->isDown = 0;
                    draw_button(hard);
                }
                Sleep(200);
                closegraph(); // 关闭菜单界面
                initgraph(30*50,16*50, EW_SHOWCONSOLE);
                int map[ROW][COL] = { 0 };
                init(map, 16,30,99);
                draw(map, 16,30);
                while (true) {
                    ExMessage msg;
                    while (peekmessage(&msg, EM_MOUSE)) {
                        if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN) {
                            mouseMsg(&msg, map, 16,30);
                            int ret = judge(map, msg.y / 50, msg.x / 50,16,30,99);
                            if (ret == -1 && msg.message != WM_RBUTTONDOWN) {
                                PlaySound("./images/boom.wav", NULL, SND_ASYNC | SND_FILENAME);
                                draw(map, 16,30);
                                int select = MessageBox(GetHWnd(), "noobs!", "you lose!", MB_OKCANCEL);
                                if (select == IDOK) {
                                    goto aa;
                                }
                                else {
                                    closegraph();
                                    return 0;
                                }
                            }
                            else if (ret == 1) {
                                draw(map, 16,30);
                                IMAGE win;
                                loadimage(&win, "./images/win3.jpg",1500,800);
                                putimage(0, 0, &win);
                                mciSendString("stop bgm", NULL, 0, NULL);
                                PlaySound("./images/win.wav", NULL, SND_ASYNC | SND_FILENAME);
                                Sleep(4000);
                                int select = MessageBox(GetHWnd(), "you win!", "Finish!", MB_OKCANCEL);
                                if (select == IDOK) {
                                    goto aa;
                                }
                                else {
                                    closegraph();
                                    return 0;
                                }
                            }
                        }
                    }
                    draw(map,16,30);
                }
                break;
            }
            else if (point_in_rect(m.x, m.y, out)) {
                out->isDown = 1;
                draw_button(out);
                Sleep(200);
                if (out->isDown) {
                    out->isDown = 0;
                    draw_button(out);
                }
                Sleep(200);
                exit(0);
            }
            else if (point_in_rect(m.x, m.y, bgm)) {
                bgm->isDown = 1;
                draw_button(bgm);
                Sleep(200);
                if (bgm->isDown) {
                    switch_music(musicFiles, &currentMusicIndex, musicCount); // 切换音乐
                    bgm->isDown = 0;
                    draw_button(bgm);
                }
                Sleep(200);
                
            }
            break;
        }
    }
    free(easy->pImageFileUp);
    free(easy->pImageFileDown);
    free(easy);
    free(normal->pImageFileUp);
    free(normal->pImageFileDown);
    free(normal);
    free(hard->pImageFileUp);
    free(hard->pImageFileDown);
    free(hard);
    free(out->pImageFileUp);
    free(out->pImageFileDown);
    free(out);
    free(bgm->pImageFileUp);
    free(bgm->pImageFileDown);
    free(bgm);
    free(next1->pImageFileUp);
    free(next1->pImageFileDown);
    free(next1);
    free(out1->pImageFileUp);
    free(out1->pImageFileDown);
    free(out1);
    closegraph();
    return 0;
}



void show(int map[][COL], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int k = 0; k < cols; k++) {
            printf("%2d ", map[i][k]);
        }
        printf("\n");
    }
}

void init(int map[][COL], int rows, int cols, int mine_num) {
    loadResources();

    for (int i = 0; i < rows; i++) {
        for (int k = 0; k < cols; k++) {
            map[i][k] = 0;
        }
    }
    for (int i = 0; i < mine_num; i++) {
        int r = rand() % rows;
        int c = rand() % cols;
        if (map[r][c] == -1) i--;
        map[r][c] = -1;
    }
    for (int i = 0; i < rows; i++) {
        for (int k = 0; k < cols; k++) {
            if (map[i][k] == -1) {
                for (int r = i - 1; r <= i + 1; r++) {
                    for (int c = k - 1; c <= k + 1; c++) {
                        if (r >= 0 && r < rows && c >= 0 && c < cols && map[r][c] != -1) {
                            map[r][c]++;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < rows; i++) {
        for (int k = 0; k < cols; k++) {
            map[i][k] += 20;
        }
    }
}

void draw(int map[][COL], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int k = 0; k < cols; k++) {
            if (map[i][k] >= 0 && map[i][k] <= 8) {
                putimage(k * 50, i * 50, &imgs[map[i][k]]);
            }
            else if (map[i][k] == -1) {
                putimage(k * 50, i * 50, &imgs[9]);
            }
            else if (map[i][k] >= 19 && map[i][k] <= 28) {
                putimage(k * 50, i * 50, &imgs[10]);
            }
            else if (map[i][k] >= 39) {
                putimage(k * 50, i * 50, &imgs[11]);
            }
        }
    }
}

void mouseMsg(ExMessage* msg, int map[][COL], int rows, int cols) {
    int c = msg->x / 50;
    int r = msg->y / 50;
    if (r >= 0 && r < rows && c >= 0 && c < cols) { // 边界检查
        if (msg->message == WM_LBUTTONDOWN) {
            if (map[r][c] >= 19 && map[r][c] <= 28) {
                PlaySound("./images/click.wav", NULL, SND_ASYNC | SND_FILENAME);
                map[r][c] -= 20;
                boomspace(map, r, c, rows, cols);
            }
        }
        else if (msg->message == WM_RBUTTONDOWN) {
            if (map[r][c] >= 19 && map[r][c] <= 28) {
                PlaySound("./images/sign.wav", NULL, SND_ASYNC | SND_FILENAME);
                map[r][c] += 20;
            }
            else if (map[r][c] >= 39) {
                map[r][c] -= 20;
            }
        }
    }
}

void boomspace(int map[][COL], int row, int col, int rows, int cols) {
    if (map[row][col] == 0) {
        for (int r = row - 1; r <= row + 1; r++) {
            for (int c = col - 1; c <= col + 1; c++) {
                if (r >= 0 && r < rows && c >= 0 && c < cols && map[r][c] >= 19 && map[r][c] <= 28) {
                    map[r][c] -= 20;
                    boomspace(map, r, c, rows, cols);
                }
            }
        }
    }
}

int judge(int map[][COL], int row, int col, int rows, int cols, int mine_num) {
    if (map[row][col] == -1 || map[row][col] == 19) {
        return -1;
    }
    int cnt = 0;
    for (int i = 0; i < rows; i++) {
        for (int k = 0; k < cols; k++) {
            if (map[i][k] >= 0 && map[i][k] <= 8) {
                cnt++;
            }
        }
    }
    if (rows * cols - mine_num == cnt) {
        return 1;
    }
    return 0;
}