#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <ncursesw/ncurses.h>
#include <locale.h>

using namespace std;

#define UP KEY_UP //上
#define DOWN KEY_DOWN //下
#define LEFT KEY_LEFT //左
#define RIGHT KEY_RIGHT //右

int n = 4; //n用来记录蛇身长度,初始为2节
int game_level;//用来记录关卡
int snake_velocity;//用来给记录蛇的移动速度
int t1, t2, t3 = 0; //用来记录以用时间
int HP = 5; //记录蛇的生命值,初始化为6
int food = 0; //用来记录所吃到的食物数
int food_x = 12, food_y = 12; //记录食物所在地

struct snake {
    int x;//蛇身所在横坐标
    int y;//蛇身所在纵坐标
    int direction;//行走方向
} snakeInfo[81];


struct map {
    int food;//此map[x][y]处是否有食物有的话food为1
    int star;//此map[x][y]处是否有星星的话值为1
    int barrier;//此map[x][y]处是否有障碍物有的话值为1
} map[26][22];

inline void setColor(int k)//改变输出字体的颜色
{
    attron(COLOR_PAIR(k));
}

inline int getTime()//用来计时
{
    clock_t t = clock() * 1000000 / CLOCKS_PER_SEC; //记录当前程序已用时间
    return t;
}

inline void gotoxy(int x, int y) //移动坐标
{
    move(y, x);
}

void setShowCursor(bool);//隐藏光标
void beginStartAnimation();//绘制启动画面以及隔墙
void selectLevel();//用来选择关卡并根据关卡设置蛇的移动速度
void updateData();//用来记录游戏的各种状态数据
int beginGame();//游戏运行
void updateUI();//用来随机产生障碍物以及食物和生命药水以及用来判断游戏的各种参数（小星星是否吃到，是否撞墙）
void handleKeyEvent();//用户是否操作键盘
void clearScreen();//清除屏幕


int main()
{
    setlocale(LC_ALL,"");
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    start_color();
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);

    beginStartAnimation();//绘制启动画面以及隔墙
    while (1) {
        selectLevel();//用来选择关卡并根据关卡设置蛇的移动速度
        setShowCursor(false);//隐藏光标
        if (!beginGame()) {
            break;    //游戏运行
        }
    }

    endwin();

    return 0;
}

void setShowCursor(bool k)//隐藏光标
{
    curs_set(k);
}

void beginStartAnimation()//绘制墙/绘制启动画面以及隔墙
{
    setShowCursor(false);//隐藏光标
    int i, j, z;
    clear();
    setColor(COLOR_WHITE);//调成白色
    for (z = 0; z < 20; z++) {
        if (z >= 0) {
            gotoxy(12, z);
            addstr("              ~--______-~                ~-___-~\"       ");
        }
        if (z >= 1) {
            gotoxy(12, z - 1);
            addstr("            ~-_           _-~          ~-_       _-~    ");
        }
        if (z >= 2) {
            gotoxy(12, z - 2);
            addstr("          \\     ~-____-~     _-~    ~-_    ~-_-~    / ");
        }
        if (z >= 3) {
            gotoxy(12, z - 3);
            addstr("         (     (         _-~    _--_    ~-_    _/   |  ");
        }
        if (z >= 4) {
            gotoxy(12, z - 4);
            addstr("          /    /            _-~      ~-_        |   |  ");
        }
        if (z >= 5) {
            gotoxy(12, z - 5);
            addstr("           /    /              _----_           \\  \\ ");
        }
        if (z >= 6) {
            gotoxy(12, z - 6);
            addstr("             /    /                            \\ \\   ");
        }
        if (z >= 7) {
            gotoxy(12, z - 7);
            addstr("              /    /                          \\\\     ");
        }
        if (z >= 8) {
            gotoxy(12, z - 8);
            addstr("                /    /                      \\\\       ");
        }
        if (z >= 9) {
            gotoxy(12, z - 9);
            addstr("                 /     /                   \\            ");
        }
        if (z >= 10) {
            gotoxy(12, z - 10);
            addstr("                  |     |                \\                ");
        }
        if (z >= 11) {
            gotoxy(12, z - 11);
            addstr("                 \\     \\                                 ");
        }
        if (z >= 12) {
            gotoxy(12, z - 12);
            addstr("        \\_______      \\                                  ");
        }
        if (z >= 13) {
            gotoxy(12, z - 13);
            addstr(" \\____|__________/  \\                                    ");
        }
        if (z >= 14) {
            gotoxy(12, z - 14);
            addstr("\\/     /~     \\_/ \\                                     ");
        }
        if (z >= 15) {
            gotoxy(12, z - 15);
            addstr("        _|__|  O|                                          ");
        }
        for (int k = 15; k < z; k++) {
            gotoxy(12, k - 15);
            addstr("                                                           ");
        }
        refresh();
        usleep(20000);
    }
    sleep(1);
    clear();
    setColor(COLOR_CYAN);//调整输出颜色
    for (i = 0, j = 60; i < 60; i++, j--) { //if是为了异步输出
        if (j > 20) {
            mvaddch(0, 2 * (j - 21), '*'); //输出第一行
        }
        if (i < 40) {
            mvaddch(23, 2 * i, '*'); // 输出最下面一行
        }
        if (j > 22 && j < 45) {
            mvaddch(j - 22, 78, '*'); //输出最右边列
        }
        if (j > 22 && j < 45) {
            mvaddch(i - 15, 0, '*'); //输出第一列
        }
        if (i > 37 && i < 60) {
            mvaddch(i - 37, 54, '*'); //输出中间那列
            usleep(10000);
        }
        refresh();
        usleep(20000);
    }
    gotoxy(56, 11);
    addstr("* * * * * * * * * * * *");                                          //56
    gotoxy(19, 0);
    setColor(COLOR_YELLOW);//调整输出颜色
    addstr("| | |贪 吃 蛇| | |"); //输出标题
    gotoxy(56, 2);
    addstr("已用时间：");
    gotoxy(75, 2);
    addstr("秒");
    gotoxy(56, 4);
    addstr("生命值：");
    gotoxy(56, 6);
    addstr("当前长度：");
    gotoxy(56, 8);
    addstr("已吃食物：");
    gotoxy(56, 10);
    addstr("第             关");
    gotoxy(64, 12);
    addstr("提示：");
    gotoxy(56, 13);
    addstr("向上：↑   向上：←");
    gotoxy(56, 14);
    addstr("向下：↓   向右：→");
    gotoxy(56, 15);
    addstr("暂停/开始：确定键 ");
    gotoxy(56, 16);
    addstr("重新选关 ：Esc键");
    gotoxy(64, 18);
    addstr("注意！");
    gotoxy(56, 19);
    addstr("1:撞到");
    setColor(COLOR_RED);
    addstr("*");
    setColor(COLOR_YELLOW);//调整输出颜色
    addstr("或墙生命值减一");
    gotoxy(56, 21);
    addstr("2:吃到小星星生命值加一");
    refresh();
}
void selectLevel()//用来选择关卡并根据关卡设置蛇的移动速度
{
    setShowCursor(true);//显示光标
    n = 4; //n用来记录蛇身长度,初始为3节
    HP = 6; //记录蛇的生命值,初始化为6
    snakeInfo[0].x = 6;                  //
    snakeInfo[0].y = 10;                 //
    snakeInfo[0].direction = RIGHT;      //
    snakeInfo[1].x = 4;                 //
    snakeInfo[1].y = 10;                //     初始化蛇所在位置和移动方向
    snakeInfo[1].direction = RIGHT;     //
    snakeInfo[2].x = 2;                 //
    snakeInfo[2].y = 10;                //
    snakeInfo[2].direction = RIGHT;    //
    snakeInfo[3].x = 4; ////////////////
    snakeInfo[3].y = 4; ///////////////记录蛇尾的信息
    snakeInfo[3].direction = RIGHT; ////
    while (1) {
        gotoxy(15, 3);
        addstr("请输入关数(1-6)：");
        refresh();
        game_level = getchar() - 48;
        if (game_level == 0) { //判断是否作弊
            gotoxy(15, 3);
            setColor(COLOR_RED);//变成红色
            addstr("  作弊有害智商，需谨慎");
            gotoxy(15, 5);
            setColor(COLOR_YELLOW);//变成黄色
            addstr("请输入你想要的蛇的生命值：");
            HP = getchar() - 48;
            gotoxy(15, 3);
            addstr("                      ");
            gotoxy(15, 5);
            addstr("                                    ");
            continue;//返回选关处
        }
        if (game_level < 7 && game_level > 0) {
            break;    //判断关数是否溢出
        }
        gotoxy(15, 5);
        addstr("输入错误！");
        gotoxy(32, 3);
        addstr("          ");
        refresh();
    }
    gotoxy(15, 3);
    addstr("                   ");
    switch (game_level) {
    case 1: {
        snake_velocity = 600000;    //
        break;
    }
    case 2: {
        snake_velocity = 400000;    //
        break;
    }
    case 3: {
        snake_velocity = 200000;    //    根据关数来设定蛇的移动速度
        break;
    }
    case 4: {
        snake_velocity = 150000;    //
        break;
    }
    case 5: {
        snake_velocity = 100000;    //
        break;
    }
    case 6: {
        snake_velocity = 60000;    //
        break;
    }
    }
    clearScreen();//清除屏幕
}
void updateData()//用来记录和判断游戏的各种状态数据
{
    gotoxy(66, 2);
    setColor(COLOR_RED);//调成红色
    printw("%d", t1 / 1000); //程序已用时间
    switch (game_level) {
    case 1:
        gotoxy(59, 10);
        setColor(COLOR_RED);//调成红色
        addstr("1");
        setColor(COLOR_YELLOW);//调成黄色
        addstr(" 2 3 4 5 6");
        break;
    case 2:
        gotoxy(59, 10);
        setColor(COLOR_YELLOW);//调成黄色
        addstr("1 ");
        setColor(COLOR_RED);//调成红色
        addstr("2");
        setColor(COLOR_YELLOW);//调成黄色
        addstr(" 3 4 5 6 ");
        break;
    case 3:
        gotoxy(59, 10);
        setColor(COLOR_YELLOW);//调成黄色
        addstr("1 2 ");
        setColor(COLOR_RED);//调成红色
        addstr("3");
        setColor(COLOR_YELLOW);//调成黄色
        addstr(" 4 5 6 ");
        break;
    case 4:
        gotoxy(59, 10);
        setColor(COLOR_YELLOW);//调成黄色
        addstr("1 2 3 ");
        setColor(COLOR_RED);//调成红色
        addstr("4");
        setColor(COLOR_YELLOW);//调成黄色
        addstr(" 5 6 ");
        break;
    case 5:
        gotoxy(59, 10);
        setColor(COLOR_YELLOW);//调成黄色
        addstr("1 2 3 4 ");
        setColor(COLOR_RED);//调成红色
        addstr("5");
        setColor(COLOR_YELLOW);//调成黄色
        addstr(" 6 ");
        break;
    case 6:
        gotoxy(59, 10);
        setColor(COLOR_YELLOW);//调成黄色
        addstr("1 2 3 4 5 ");
        setColor(COLOR_RED);//调成红色
        addstr("6");
        break;
    }
    switch (HP) {
    case 1:
        gotoxy(65, 4);
        setColor(COLOR_GREEN);//调成绿色
        addstr("▁");
        setColor(COLOR_RED);//调成红色
        addstr("▂▃▅▆▇");
        break;
    case 2:
        gotoxy(65, 4);
        setColor(COLOR_GREEN);//调成绿色
        addstr("▁▂");
        setColor(COLOR_RED);//调成红色
        addstr("▃▅▆▇");
        break;
    case 3:
        gotoxy(65, 4);
        setColor(COLOR_GREEN);//调成绿色
        addstr("▁▂▃");
        setColor(COLOR_RED);//调成红色
        addstr("▅▆▇");
        break;
    case 4:
        gotoxy(65, 4);
        setColor(COLOR_GREEN);//调成绿色
        addstr("▁▂▃▅");
        setColor(COLOR_RED);//调成红色
        addstr("▆▇");
        break;
    case 5:
        gotoxy(65, 4);
        setColor(COLOR_GREEN);//调成绿色
        addstr("▁▂▃▅▆");
        setColor(COLOR_RED);//调成红色
        addstr("▇");
        break;
    case 6:
        gotoxy(65, 4);
        setColor(COLOR_GREEN);//调成绿色
        addstr("▁▂▃▅▆▇");
        break;
    default:
        gotoxy(65, 4);
        setColor(COLOR_GREEN);//调成红色
        addstr("！超级模式 ！");
        break;
    }
    gotoxy(66, 6);
    setColor(COLOR_RED);//调成红色
    printw("%d", n - 1); //输出蛇的当前长度
    gotoxy(66, 8);
    printw("%d", food); //输出蛇当前已经吃到食物
    refresh();
}
void clearScreen()//用来清除屏幕
{
    for (int i = 2; i < 23; i++) {
        gotoxy(2, i);
        addstr("                                                    ");
    }
    map[food_x][food_y].food = 0; //将食物清空
    map[food_x][food_y].barrier = 0; //将障碍物清除
    map[food_x][food_y].star = 0; //将星星清除
    refresh();
}

void updateUI()//用来随机产生障碍物以及食物和生命药水以及用来判断游戏的各种参数
{
    int a, b, e, f; //a，b用来表示小星星的坐标   c，d代表障碍物坐标
    if (map[food_x][food_y].food == 0) { //判断食物是不是被吃掉
        while (1) {
            food_x = random() % 26; //产生随机横坐标
            food_y = random() % 22; //产生随机纵坐标
            if (map[food_x][food_y].barrier == 0 && map[food_x][food_y].star == 0) {
                break;    //当此处无其他元素是才生效
            }
        }
        map[food_x][food_y].food = 1; //随机出现食物
        gotoxy(2 * (food_x + 1), food_y + 1); //定位到食物出现的位置
        setColor(COLOR_YELLOW);//调成黄色
        addstr("☆"); //打印出食物
    }
    if (t1 / 20 > 0 && t1 % 12 == 0 && t1 > t3 && map[(snakeInfo[0].x - 1) / 2][snakeInfo[0].y - 1].food == 0 && map[(snakeInfo[0].x - 1) / 2][snakeInfo[0].y - 1].star == 0) {
        while (1) {
            e = random() % 26; //产生随机横坐标
            f = random() % 22; //产生随机纵坐标
            if (map[e][f].food == 0 && map[e][f].star == 0) {
                break;    //当此处无其他元素是才生效
            }
        }
        gotoxy(2 * (e + 1), f + 1); //定位到障碍物出现的位置
        map[e][f].barrier = 1; //随机出现障碍物
        setColor(COLOR_RED);//调成黄色
        addch('*'); //打印出障碍物
        t3 = t1; //以免产生多个障碍物
        if (HP < 7) {
            gotoxy(18, 24);
            setColor(COLOR_WHITE);//调成白色
            addstr("温馨提示：在选关的时候输入0可以开启作弊模式");
        }
    }
    if (t1 / 25 > 0 && t1 % 15 == 0 && t1 > t3 && map[(snakeInfo[0].x - 1) / 2][snakeInfo[0].y - 1].food == 0 && map[(snakeInfo[0].x - 1) / 2][snakeInfo[0].y - 1].barrier == 0) { //减少星星出现的几率
        while (1) {
            a = random() % 26; //产生随机横坐标
            b = random() % 22; //产生随机纵坐标
            if (map[a][b].barrier == 0 && map[a][b].food == 0) {
                break;    //当此处无其他元素是才生效
            }
        }
        map[a][b].star = 1; //随机出现小星星（吃到星星长度减1）
        gotoxy(2 * (a + 1), b + 1); //定位到星星出现的位置（吃到星星长度减1）
        setColor(COLOR_YELLOW);//调成黄色
        addstr("☆"); //打印出星星（吃到星星长度减1）
        t3 = t1; //以免产生多个障碍物
        if (HP < 7) {
            gotoxy(18, 24);
            addstr("                                            ");
        }
    }
    for (int i = 0; i < n; i++) {
        if (map[(snakeInfo[i].x - 1) / 2][snakeInfo[i].y - 1].food == 1) { //判断蛇是否吃到食物
            ++n;//让蛇长度加1
            food++;//将食物数加1
            map[(snakeInfo[i].x - 1) / 2][snakeInfo[i].y - 1].food = 0; //让食物标示归零
            break;
        }
    }
    if (map[(snakeInfo[0].x - 1) / 2][snakeInfo[0].y - 1].star == 1) { //判断蛇是否吃到星星
        map[(snakeInfo[0].x - 1) / 2][snakeInfo[0].y - 1].star = 0; //让星星标示归零
        if (HP < 6) {
            ++HP;    //将生命值加1
        }
    }
    t1 = getTime() - t2; //刷新游戏运行时间
    refresh();
}
void handleKeyEvent()//用户是否操作键盘
{
    halfdelay(1);
    int ch = getch();
    if ((ch == UP || ch == DOWN || ch == LEFT || ch == RIGHT) && (ch + snakeInfo[0].direction != UP+DOWN) && (ch + snakeInfo[0].direction != LEFT+RIGHT) && ch != snakeInfo[0].direction) { //判断按键是否是方向键，并且是不是蛇移动方向的反方向
        snakeInfo[0].direction = ch;    //如果不是就改变蛇头方向
    } else if (ch == 10) { //判断用户是否暂停
        clock_t a, b;
        a = getTime(); //记录当前程序已用时间
        gotoxy(20, 1);
        setColor(COLOR_WHITE);//调成白色
        addstr("已暂停,按确定键开始");
        while (1) {
            if (getch() == 10) { ////判断是否按键且是否解除暂停
                gotoxy(20, 1);
                addstr("                     "); //清除"已暂停,按确定键开始"这行字
                break;
            }
        }
        b = getTime(); //记录当前程序已用时间
        t2 += (b - a); //将暂停加到t2上供t1减去
    } else if (ch == 27) { //判断是否重新选关
        selectLevel();//用来选择关卡并根据关卡设置蛇的移动速度
        beginGame();//开始游戏
    }
    halfdelay(0);
}
int beginGame()
{
    setShowCursor(false);
    int i;
    //int ch = RIGHT; //向右
    t2 = getTime(); //记录当前程序已用时间
    while (1) {
        t1 = getTime() - t2; //刷新游戏运行时间
        updateData();//用来记录游戏的各种状态数据
        gotoxy(snakeInfo[0].x, snakeInfo[0].y); //转到蛇头位置
        setColor(COLOR_RED);//改成红色
        addstr("◆"); //打印蛇头
        for (i = 1; i < n - 1; i++) {
            gotoxy(snakeInfo[i].x, snakeInfo[i].y); //转到当前蛇身位置
            setColor(COLOR_YELLOW);//改成黄色
            addstr("●"); //打印蛇身
        }
        gotoxy(snakeInfo[n - 2].x, snakeInfo[n - 2].y); //转到当前蛇尾位置
        setColor(COLOR_RED);//改成红色
        addstr("●"); //打印蛇尾
        usleep(snake_velocity);//控制蛇的移动速度
        t1 = getTime() - t2; //刷新游戏运行时间
        gotoxy(snakeInfo[n - 2].x, snakeInfo[n - 2].y); //移到蛇尾所在地
        addstr(" "); //清除上个循环的蛇尾
        for (i = n - 1; i > 0; i--) {
            snakeInfo[i] = snakeInfo[i - 1];    //移动蛇
        }
        handleKeyEvent();//用户是否操作键盘
        switch (snakeInfo[0].direction) {
        case UP: {
            snakeInfo[0].y -= 1;    //改变蛇头坐标，移动蛇头
            break;
        }
        case DOWN: {
            snakeInfo[0].y += 1;    //改变蛇头坐标，移动蛇头
            break;
        }
        case LEFT: {
            snakeInfo[0].x -= 2;    //改变蛇头坐标，移动蛇头
            break;
        }
        case RIGHT: {
            snakeInfo[0].x += 2;    //改变蛇头坐标，移动蛇头
            break;
        }
        }
        if (snakeInfo[0].x == 0) { //当蛇撞到左墙时
            --HP;//将生命值减一
            snakeInfo[0].x = 52; //将其穿墙
        }
        if (snakeInfo[0].x == 54) { //当蛇撞到右墙时
            --HP;//将生命值减一
            snakeInfo[0].x = 2; //将其穿墙
        }
        if (snakeInfo[0].y == 0) { //当蛇撞到上墙时
            --HP;//将生命值减一
            snakeInfo[0].y = 22; //将其穿墙
        }
        if (snakeInfo[0].y == 23) { //当蛇撞到下墙时
            --HP;//将生命值减一
            snakeInfo[0].y = 1; //将其穿墙
        }
        for (i = 1; i < n - 1; i++) {
            if (snakeInfo[0].x == snakeInfo[i].x && snakeInfo[0].y == snakeInfo[i].y) {
                i = n + 1;    //判断蛇是否撞到自
            }
        }
        if (i >= n) { //当蛇撞到自己
            HP = 0; //将蛇死亡
        }
        if (map[(snakeInfo[0].x - 1) / 2][snakeInfo[0].y - 1].barrier == 1) { //当蛇障碍物时
            --HP;//将生命值减一
            map[(snakeInfo[0].x - 1) / 2][snakeInfo[0].y - 1].barrier = 0;
        }
        if (HP == 0) {
            gotoxy(25, 5);
            setColor(COLOR_WHITE);//调成白色
            addstr("游戏结束！！！");
            usleep(3000);//延时
            return 1;
            break;
        }
        if (n == 81) {
            gotoxy(25, 5);
            setColor(COLOR_WHITE);//调成白色
            addstr("恭喜你过关！！！");
            usleep(3000);//延时
            return 1;
            break;
        }
        updateUI();//用来随机产生障碍物以及食物和生命药水以及用来判断游戏的各种参数（小星星是否吃到，是否撞墙)
    }
    return 0;
}
