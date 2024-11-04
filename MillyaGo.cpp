#pragma GCC optimize(3,"Ofast","inline")
#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <chrono>

#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2
#define TRUE 1
#define FALSE 0
#define START "START"
#define PLACE "PLACE"
#define TURN "TURN"
#define END "END"

using namespace std;
using namespace chrono;

/*写在最前面的声明：
 *请不要用AI给我写审阅，谢谢
 *这份代码虽说不算很强，但是也算是我四个星期的心血
 *请不要用AI为我生成一份前言不搭后语的，严重曲解代码意思的评估
 *这是对我，也是对这份代码的不尊重
 *如果有不理解的地方或者有改进建议，欢迎单独找我聊
 *我超和善
 *谢谢
*/

int MAX_DEPTH;  // 最大搜索深度
int VCF_MAX_DEPTH = 16;  // VCF最大搜索深度
int VCT_MAX_DEPTH = 6;  // VCT最大搜索深度
int alphaBetaCount = 0;  // alphabeta计数统计
int zobristCount = 0;  // zobrist替换计数统计
int evalCount = 0;  // 估值次数计数统计
double evaluateTime = 0;  // 估值时间统计
double checkBoardTime = 0;  // 检查局面时间统计
double getMovesTime = 0;  // 获取可行步数统计

steady_clock::time_point testStart;
bool DEBUG_MODE = true;  // 本地debug，打印棋盘和相关统计信息
bool TIME_LIMIT = true;  // 设置定时

bool timeLimitBreak = false;
bool cuttedMoves = false;
steady_clock::time_point globalStartTime;

double getDeltaTime(steady_clock::time_point s) {
    // 获取时间
    duration<double> timeSpan = duration_cast<duration<double>>(steady_clock::now() - s);
    return timeSpan.count();
}

struct Command {
    int x;
    int y;
    Command() {};
    Command(int x_i, int y_i) {
        x = x_i;
        y = y_i;
    }
};

vector<Command> onBoardCommands;  // 目前在板上的Commands

struct CommandVectorNode {
    Command com;
    int pointValue;
};

class Zobrist {
    /*Zobrist置换表
     *具体参见：https://www.bookstack.cn/read/lihongxun945-gobang-ai/fddd888addab81b9.md
     *总体而言，就是将某个局面给哈希成一个值，从而辅助记录
     *以下data的“随机值”是预先生成过后不变的，在此处写死
     *代码中开局库的实现也是基于Zobrist与下面的随机值
     *开局库的生成有单独的另一份Python代码，欢迎联系我获取
     */
public:
    int value = 0;
    Zobrist() {
        data[0][0][0] = 165729349; data[0][0][1] = 505513791; data[0][1][0] = -831957664; data[0][1][1] = 1007757579; data[0][2][0] = 662676825; data[0][2][1] = -629087639; data[0][3][0] = -1552276576; data[0][3][1] = -155485281; data[0][4][0] = -115687006; data[0][4][1] = -1981310658; data[0][5][0] = -91465145; data[0][5][1] = -2128873240; data[0][6][0] = 1709766319; data[0][6][1] = 1023408723; data[0][7][0] = -1707693725; data[0][7][1] = -2095882449; data[0][8][0] = -1812293191; data[0][8][1] = -1516274531; data[0][9][0] = -403922771; data[0][9][1] = 178126970; data[0][10][0] = -2101064783; data[0][10][1] = -49985292; data[0][11][0] = -649623054; data[0][11][1] = 1962132230; data[1][0][0] = -2098913141; data[1][0][1] = 1736439612; data[1][1][0] = 1016319746; data[1][1][1] = -1526381928; data[1][2][0] = -262853130; data[1][2][1] = 975289080; data[1][3][0] = 504265311; data[1][3][1] = -300430525; data[1][4][0] = 1018947330; data[1][4][1] = 1864143185; data[1][5][0] = -190626171; data[1][5][1] = -884601289; data[1][6][0] = 474710867; data[1][6][1] = -93526692; data[1][7][0] = -301410550; data[1][7][1] = 419048499; data[1][8][0] = 473386763; data[1][8][1] = 1308081620; data[1][9][0] = -664090615; data[1][9][1] = 72133735; data[1][10][0] = -15994348; data[1][10][1] = 796832256; data[1][11][0] = 160170529; data[1][11][1] = -1676120462; data[2][0][0] = -682261036; data[2][0][1] = -629531015; data[2][1][0] = 1619630352; data[2][1][1] = 292558732; data[2][2][0] = 844091254; data[2][2][1] = 227226630; data[2][3][0] = -211491059; data[2][3][1] = -2344102; data[2][4][0] = -518349991; data[2][4][1] = 1903361661; data[2][5][0] = 109377347; data[2][5][1] = 1899250357; data[2][6][0] = 914050998; data[2][6][1] = 61157747; data[2][7][0] = 1523443462; data[2][7][1] = 1750866577; data[2][8][0] = -272772592; data[2][8][1] = 778929572; data[2][9][0] = 203100901; data[2][9][1] = 1492710297; data[2][10][0] = -1953101895; data[2][10][1] = 1663602283; data[2][11][0] = -1199203351; data[2][11][1] = -151026979; data[3][0][0] = 407098601; data[3][0][1] = 520903922; data[3][1][0] = 463385711; data[3][1][1] = -1109711232; data[3][2][0] = -2063714289; data[3][2][1] = 905842817; data[3][3][0] = -1597287419; data[3][3][1] = 1139474533; data[3][4][0] = 2008214852; data[3][4][1] = -1414932928; data[3][5][0] = -2072892677; data[3][5][1] = 387141536; data[3][6][0] = 1894393247; data[3][6][1] = -365441808; data[3][7][0] = 583169649; data[3][7][1] = 839562292; data[3][8][0] = 132887967; data[3][8][1] = 1143927657; data[3][9][0] = 871960399; data[3][9][1] = -1858792171; data[3][10][0] = 148329078; data[3][10][1] = -651851924; data[3][11][0] = 251186422; data[3][11][1] = -1798522464; data[4][0][0] = -1284943805; data[4][0][1] = -1004228384; data[4][1][0] = 1997806401; data[4][1][1] = -1915317165; data[4][2][0] = -1081291425; data[4][2][1] = 1140378824; data[4][3][0] = 764425395; data[4][3][1] = -653006955; data[4][4][0] = -620899522; data[4][4][1] = 1731232822; data[4][5][0] = 1132042488; data[4][5][1] = -507085924; data[4][6][0] = -279852419; data[4][6][1] = 2058530656; data[4][7][0] = -1887371884; data[4][7][1] = -1833435763; data[4][8][0] = 1626591654; data[4][8][1] = -840677886; data[4][9][0] = -1245532643; data[4][9][1] = -972124648; data[4][10][0] = -580593121; data[4][10][1] = -423726691; data[4][11][0] = 1346679522; data[4][11][1] = 541186430; data[5][0][0] = -1770398660; data[5][0][1] = -76350403; data[5][1][0] = -1278153516; data[5][1][1] = 1957987642; data[5][2][0] = -1554005322; data[5][2][1] = -1610508299; data[5][3][0] = -282253486; data[5][3][1] = 1958707438; data[5][4][0] = 1859949149; data[5][4][1] = -526178563; data[5][5][0] = -476744593; data[5][5][1] = 1275891388; data[5][6][0] = -1344236205; data[5][6][1] = -336206489; data[5][7][0] = 1939411742; data[5][7][1] = -1326261260; data[5][8][0] = -314716591; data[5][8][1] = -1173686693; data[5][9][0] = 25120690; data[5][9][1] = 2064640323; data[5][10][0] = 977332931; data[5][10][1] = 384713676; data[5][11][0] = -1458048206; data[5][11][1] = 193875148; data[6][0][0] = 1887073593; data[6][0][1] = 1399757568; data[6][1][0] = -1978046997; data[6][1][1] = -1912531627; data[6][2][0] = -274407214; data[6][2][1] = 1672611062; data[6][3][0] = 1782999396; data[6][3][1] = 287409697; data[6][4][0] = -589742287; data[6][4][1] = 798230762; data[6][5][0] = 320822673; data[6][5][1] = 1381816289; data[6][6][0] = -994269527; data[6][6][1] = 1129891567; data[6][7][0] = 1891349257; data[6][7][1] = -1280858436; data[6][8][0] = 1818899702; data[6][8][1] = 556159298; data[6][9][0] = 1795916674; data[6][9][1] = -906877484; data[6][10][0] = -556861696; data[6][10][1] = -413658694; data[6][11][0] = 2023383405; data[6][11][1] = -2034892637; data[7][0][0] = 967623325; data[7][0][1] = 1267258891; data[7][1][0] = 1164656596; data[7][1][1] = 2084960242; data[7][2][0] = -36077174; data[7][2][1] = 1243882946; data[7][3][0] = 1607580645; data[7][3][1] = -51820742; data[7][4][0] = -913030234; data[7][4][1] = -138267596; data[7][5][0] = -2104702161; data[7][5][1] = -132993215; data[7][6][0] = 1698781153; data[7][6][1] = 767592623; data[7][7][0] = -403536200; data[7][7][1] = -1094892195; data[7][8][0] = 2100496608; data[7][8][1] = -1184939707; data[7][9][0] = -392849148; data[7][9][1] = 1367652350; data[7][10][0] = 698452024; data[7][10][1] = -49805412; data[7][11][0] = -159260988; data[7][11][1] = -1343985906; data[8][0][0] = -1313845850; data[8][0][1] = -1772162500; data[8][1][0] = 817415714; data[8][1][1] = -569349413; data[8][2][0] = -527373034; data[8][2][1] = -1986805953; data[8][3][0] = -145223055; data[8][3][1] = -1080154162; data[8][4][0] = 1823175036; data[8][4][1] = -1352584156; data[8][5][0] = -2058338829; data[8][5][1] = 1402376872; data[8][6][0] = -1802067794; data[8][6][1] = -1401803289; data[8][7][0] = -1826088729; data[8][7][1] = 1363454267; data[8][8][0] = 10895547; data[8][8][1] = 1932205359; data[8][9][0] = 1037504626; data[8][9][1] = 602282402; data[8][10][0] = -1712420840; data[8][10][1] = -1366571567; data[8][11][0] = 1372874208; data[8][11][1] = -1272113475; data[9][0][0] = 962714295; data[9][0][1] = -2090710860; data[9][1][0] = 469024170; data[9][1][1] = -979514657; data[9][2][0] = 1823471564; data[9][2][1] = -1784995038; data[9][3][0] = 2070855689; data[9][3][1] = -393133479; data[9][4][0] = -2022899247; data[9][4][1] = -1751388410; data[9][5][0] = 60807473; data[9][5][1] = -992124752; data[9][6][0] = 2082042580; data[9][6][1] = 802527620; data[9][7][0] = -1471990446; data[9][7][1] = 1094799302; data[9][8][0] = 65931993; data[9][8][1] = 1979392195; data[9][9][0] = 213058997; data[9][9][1] = 1023783556; data[9][10][0] = 509946320; data[9][10][1] = 1056168989; data[9][11][0] = -869730279; data[9][11][1] = -1292752523; data[10][0][0] = -216960465; data[10][0][1] = 469119370; data[10][1][0] = 1521990044; data[10][1][1] = -2017060113; data[10][2][0] = -1949832807; data[10][2][1] = -866920385; data[10][3][0] = -1185276628; data[10][3][1] = -1525437793; data[10][4][0] = -395726696; data[10][4][1] = -1466199371; data[10][5][0] = -1900707344; data[10][5][1] = 1325764743; data[10][6][0] = 314961828; data[10][6][1] = -170678975; data[10][7][0] = -381591347; data[10][7][1] = -500528190; data[10][8][0] = -644788390; data[10][8][1] = 1288843451; data[10][9][0] = 668534347; data[10][9][1] = 346464784; data[10][10][0] = 1333834991; data[10][10][1] = -99805611; data[10][11][0] = -326530283; data[10][11][1] = -1615982401; data[11][0][0] = 710200433; data[11][0][1] = -75220844; data[11][1][0] = 1137484808; data[11][1][1] = -2062586935; data[11][2][0] = -1406931504; data[11][2][1] = 1564570974; data[11][3][0] = 890409678; data[11][3][1] = 1674583643; data[11][4][0] = 1328577901; data[11][4][1] = -1331820888; data[11][5][0] = 116767774; data[11][5][1] = -631028519; data[11][6][0] = 762777967; data[11][6][1] = 439034460; data[11][7][0] = -942288079; data[11][7][1] = 997199964; data[11][8][0] = 1697117846; data[11][8][1] = 205940039; data[11][9][0] = 990087783; data[11][9][1] = 765363429; data[11][10][0] = -740914708; data[11][10][1] = -625204974; data[11][11][0] = 1423107139; data[11][11][1] = 419346011;
    }

    void add(Command c, int player) {
        value ^= data[c.x][c.y][player - 1];
    }

    void record(int s, int depth) {
        pair<int, int> p(s, depth);
        score[value] = p;
    }

    bool isExist() {
        return score.find(value) != score.end();
    }

    int data[BOARD_SIZE][BOARD_SIZE][2];
    unordered_map<int, pair<int, int>> score;
};

Zobrist zobrist;
unordered_map<int, pair<int, int>> opening;
unordered_map<unsigned int, int> scoreMap;

char board[BOARD_SIZE][BOARD_SIZE] = { 0 };
string boardLine[4][2 * BOARD_SIZE + 1];  // 单独储存某一条线的字符串
int myFlag;
int enemyFlag;
Command enemyLastMove = Command();

int defenseFirstRate = 3;  // 先手防守倍率
int defenseSecondRate = 5;  // 后手防守倍率
int range = 2;

int evaluationValue;

steady_clock::time_point startTime;

struct Shape {
    int score;
    char shape[10];
};

Shape Score_Shape[30] = {
    {50,"01100"},
    {50,"00110"},
    {200,"11010"},
    {500,"00111"},
    {500,"11100"},
    {7000,"01110"},
    {5000,"010110"},
    {5000,"011010"},
    {5000,"11101"},
    {5000,"11011"},
    {5000,"10111"},
    {6000,"11110"},
    {6000,"01111"},
    {500000,"011110"},
    {99999999,"11111"},
    {-50,"02200"},
    {-50,"00220"},
    {-200,"22020"},
    {-500,"00222"},
    {-500,"22200"},
    {-7000,"02220"},
    {-5000,"020220"},
    {-5000,"022020"},
    {-5000,"22202"},
    {-5000,"22022"},
    {-5000,"20222"},
    {-6000,"22220"},
    {-6000,"02222"},
    {-500000,"022220"},
    {-99999999,"22222"}
};

char k4Patterns[2][5][6] = { { "01111", "10111", "11011", "11101", "11110" },
                            {"02222", "20222", "22022", "22202", "22220"} };

char h3Patterns[2][4][7] = { {"011100", "001110", "010110", "011010"},
                        {"022200", "002220", "020220", "022020"} };

void debug(const char* str) {
    printf("DEBUG %s\n", str);
    fflush(stdout);
}

bool isInBound(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

inline void put(struct Command cmd, int flag) {
    // 下子
    zobrist.add(cmd, flag);  // 更新zobrist值

    // 将该子记录在棋盘上
    char ch;
    if (flag == 1) ch = '1';
    else ch = '2';
    board[cmd.x][cmd.y] = flag;
    boardLine[0][cmd.x][cmd.y] = ch;
    boardLine[1][cmd.y][cmd.x] = ch;

    // 更新对应直线的缓存存储
    if (cmd.x - cmd.y <= 0) {
        boardLine[2][cmd.x - cmd.y + 11][cmd.x] = ch;
    }
    else {
        boardLine[2][cmd.x - cmd.y + 11][cmd.y] = ch;
    }
    if (cmd.x + cmd.y <= 11) {
        boardLine[3][cmd.x + cmd.y][cmd.y] = ch;
    }
    else {
        boardLine[3][cmd.x + cmd.y][11 - cmd.x] = ch;
    }

    onBoardCommands.emplace_back(cmd);
}

inline void unput(struct Command cmd) {
    // 取消下子，流程同上
    zobrist.add(cmd, board[cmd.x][cmd.y]);
    board[cmd.x][cmd.y] = EMPTY;
    boardLine[0][cmd.x][cmd.y] = '0';
    boardLine[1][cmd.y][cmd.x] = '0';
    if (cmd.x - cmd.y <= 0) {
        boardLine[2][cmd.x - cmd.y + 11][cmd.x] = '0';
    }
    else {
        boardLine[2][cmd.x - cmd.y + 11][cmd.y] = '0';
    }
    if (cmd.x + cmd.y <= 11) {
        boardLine[3][cmd.x + cmd.y][cmd.y] = '0';
    }
    else {
        boardLine[3][cmd.x + cmd.y][11 - cmd.x] = '0';
    }
    onBoardCommands.pop_back();
}

inline void place(struct Command cmd) {
    // 同put，历史遗留原因不想更改函数名，便加一层封装
    put(cmd, enemyFlag);
}

void printBoard() {
    // 测试用，打印棋盘
    int width = 4;
    cout << setw(width) << ' ';
    for (int i = 0; i < BOARD_SIZE; ++i) cout << setw(width) << i;
    cout << endl;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        cout << setw(width) << i;
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] == EMPTY) cout << setw(width) << '.';
            else if (board[i][j] == BLACK) cout << setw(width) << 'X';
            else if (board[i][j] == WHITE) cout << setw(width) << 'O';
        }
        printf("\n");
    }
    printf("\n");
}

int checkGameStatus() {
    // 检查棋局状态，1则我方胜利，-1则敌方胜利，0则未有结果
    testStart = steady_clock::now();

    // 检查横向、纵向、和两个对角线是否有五子连珠
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (j + 4 < BOARD_SIZE &&
                (board[i][j] == myFlag && board[i][j + 1] == myFlag &&
                    board[i][j + 2] == myFlag && board[i][j + 3] == myFlag &&
                    board[i][j + 4] == myFlag)) {
                return 1;
            }

            if (i + 4 < BOARD_SIZE &&
                (board[i][j] == myFlag && board[i + 1][j] == myFlag &&
                    board[i + 2][j] == myFlag && board[i + 3][j] == myFlag &&
                    board[i + 4][j] == myFlag)) {
                return 1;
            }

            if (i + 4 < BOARD_SIZE && j + 4 < BOARD_SIZE &&
                (board[i][j] == myFlag && board[i + 1][j + 1] == myFlag &&
                    board[i + 2][j + 2] == myFlag && board[i + 3][j + 3] == myFlag &&
                    board[i + 4][j + 4] == myFlag)) {
                return 1;
            }

            if (i + 4 < BOARD_SIZE && j - 4 >= 0 &&
                (board[i][j] == myFlag && board[i + 1][j - 1] == myFlag &&
                    board[i + 2][j - 2] == myFlag && board[i + 3][j - 3] == myFlag &&
                    board[i + 4][j - 4] == myFlag)) {
                return 1;
            }

            if (j + 4 < BOARD_SIZE &&
                (board[i][j] == enemyFlag && board[i][j + 1] == enemyFlag &&
                    board[i][j + 2] == enemyFlag && board[i][j + 3] == enemyFlag &&
                    board[i][j + 4] == enemyFlag)) {
                return -1;
            }

            if (i + 4 < BOARD_SIZE &&
                (board[i][j] == enemyFlag && board[i + 1][j] == enemyFlag &&
                    board[i + 2][j] == enemyFlag && board[i + 3][j] == enemyFlag &&
                    board[i + 4][j] == enemyFlag)) {
                return -1;
            }

            if (i + 4 < BOARD_SIZE && j + 4 < BOARD_SIZE &&
                (board[i][j] == enemyFlag && board[i + 1][j + 1] == enemyFlag &&
                    board[i + 2][j + 2] == enemyFlag && board[i + 3][j + 3] == enemyFlag &&
                    board[i + 4][j + 4] == enemyFlag)) {
                return -1;
            }

            if (i + 4 < BOARD_SIZE && j - 4 >= 0 &&
                (board[i][j] == enemyFlag && board[i + 1][j - 1] == enemyFlag &&
                    board[i + 2][j - 2] == enemyFlag && board[i + 3][j - 3] == enemyFlag &&
                    board[i + 4][j - 4] == enemyFlag)) {
                return -1;
            }
        }
    }

    checkBoardTime += getDeltaTime(testStart);

    return 0;
}

int checkGameStatus(Command c) {
    // 同上，但是接受一个Command，只检查这个Command所在的直线，更快
    testStart = steady_clock::now();

    // 横向
    int count = 1;
    int player = board[c.x][c.y];
    for (int k = 1, emptyStart = 0; isInBound(c.x - k, c.y) && board[c.x - k][c.y] == player; ++k) {
        if (board[c.x - k][c.y] == player) ++count;
    }
    for (int k = 1, emptyStart = 0; isInBound(c.x - k, c.y) && board[c.x + k][c.y] == player; ++k) {
        if (board[c.x + k][c.y] == player) ++count;
    }
    if (count == 5) return 1;

    // 纵向
    count = 1;
    for (int k = 1, emptyStart = 0; isInBound(c.x, c.y - k) && board[c.x][c.y - k] == player; ++k) {
        if (board[c.x][c.y - k] == player) ++count;
    }
    for (int k = 1, emptyStart = 0; isInBound(c.x, c.y + k) && board[c.x][c.y + k] == player; ++k) {
        if (board[c.x][c.y + k] == player) ++count;
    }
    if (count == 5) return 1;

    // 主对角线
    count = 1;
    for (int k = 1, emptyStart = 0; isInBound(c.x - k, c.y - k) && board[c.x - k][c.y - k] == player; ++k) {
        if (board[c.x - k][c.y - k] == player) ++count;
    }
    for (int k = 1, emptyStart = 0; isInBound(c.x + k, c.y + k) && board[c.x + k][c.y + k] == player; ++k) {
        if (board[c.x + k][c.y + k] == player) ++count;
    }
    if (count == 5) return 1;

    // 副对角线
    count = 1;
    for (int k = 1, emptyStart = 0; isInBound(c.x - k, c.y + k) && board[c.x - k][c.y + k] != player; ++k) {
        if (board[c.x - k][c.y + k] == player) ++count;
    }
    for (int k = 1, emptyStart = 0; isInBound(c.x + k, c.y - k) && board[c.x + k][c.y - k] != player; ++k) {
        if (board[c.x + k][c.y - k] == player) ++count;
    }
    if (count == 5) return 1;

    return 0;
}

void initAI(int me) {
    enemyFlag = 3 - me;
}

void getAllScore();

unsigned int bkdrHash(const string key) {
    // str --> unsigned int 的哈希
    char* str = const_cast<char*>(key.c_str());

    unsigned int seed = 31; // 31 131 1313 13131 131313 etc.. 37
    unsigned int hash = 0;
    while (*str) {
        hash = hash * seed + (*str++);
    }
    return hash;
}

char* transform(int type, int x, int y);

int scoreEvaluation(char* str);

void start(int flag) {
    // 设置棋盘初始状态
    memset(board, 0, sizeof(board));
    int middlePlace = BOARD_SIZE / 2;
    board[middlePlace - 1][middlePlace - 1] = WHITE;
    board[middlePlace][middlePlace] = WHITE;
    board[middlePlace - 1][middlePlace] = BLACK;
    board[middlePlace][middlePlace - 1] = BLACK;

    // 设置每条直线缓存的初始状态
    for (int i = 0; i < BOARD_SIZE; ++i) {
        boardLine[0][i] = transform(1, i, i);
        boardLine[1][i] = transform(2, i, i);
    }
    for (int i = 0; i < 2 * BOARD_SIZE - 1; ++i) {
        if (i <= 11) {
            boardLine[2][i] = transform(3, 0, 11 - i);
        }
        else {
            boardLine[2][i] = transform(3, i - 11, 0);
        }
        if (i <= 11) {
            boardLine[3][i] = transform(4, 0, i);
        }
        else {
            boardLine[3][i] = transform(4, i - 11, 11);
        }
    }

    onBoardCommands.emplace_back(Command(5, 5));
    onBoardCommands.emplace_back(Command(5, 6));
    onBoardCommands.emplace_back(Command(6, 5));
    onBoardCommands.emplace_back(Command(6, 6));
    initAI(flag);

    getAllScore();

    // TEST
}

bool cVectorCmp(CommandVectorNode a, CommandVectorNode b) {
    // 给CommandVectorNode排序用，估值大优先，否则离棋盘中点近的优先
    if (a.pointValue != b.pointValue) return a.pointValue > b.pointValue;
    return ((a.com.x - 5) * (a.com.x - 5) + (a.com.y - 5) * (a.com.y - 5) < (b.com.x - 5) * (b.com.x - 5) + (b.com.y - 5) * (b.com.y - 5));
}

int findSubStrNum(char* str, char* sub_str) {
    // 寻找字串个数，用于匹配棋形
    char* str_local = NULL;
    char* sub_str_local = NULL;
    int num = 0;

    while (*str) {
        str_local = str;
        sub_str_local = sub_str;
        do {
            if (*sub_str_local == '\0') {
                num++;
                break;
            }
        } while (*str_local++ == *sub_str_local++);
        str += 1;
    }
    return num;
}

bool findSubStr(char* str, char* sub_str) {
    // 寻找字串，不管个数
    char* str_local = NULL;
    char* sub_str_local = NULL;

    while (*str) {
        str_local = str;
        sub_str_local = sub_str;
        do {
            if (*sub_str_local == '\0') {
                return true;
            }
        } while (*str_local++ == *sub_str_local++);
        str += 1;
    }
    return false;
}

char* transform(int type, int x, int y) {
    // 获取棋盘上的某条直线，转换为char数组
    int i;
    static char result[BOARD_SIZE + 1];
    if (type == 1) {
        for (i = 0; i < BOARD_SIZE; i++) {
            result[i] = board[x][i] + '0';
        }
        result[i] = '\0';
        return result;
    }
    else if (type == 2) {
        for (i = 0; i < BOARD_SIZE; i++) {
            result[i] = board[i][y] + '0';
        }
        result[i] = '\0';
        return result;
    }
    else if (type == 3) {
        for (i = 0; (i + x < BOARD_SIZE && i + y < BOARD_SIZE); i++) {
            result[i] = board[x + i][y + i] + '0';
        }
        result[i] = '\0';
        return result;
    }
    else if (type == 4) {
        for (i = 0; (x + i < BOARD_SIZE && y - i >= 0); i++) {
            result[i] = board[x + i][y - i] + '0';
        }
        result[i] = '\0';
        return result;
    }
}

char* rangedTransform(int type, int x, int y) {
    // 同上，但仅以x，y为中心最多半径为5的直线
    int i;
    int index = 0;
    static char result[12];
    if (type == 1) {
        for (i = max(y - 4, 0); i <= min(y + 4, BOARD_SIZE - 1); i++) {
            result[index] = board[x][i] + '0';
            ++index;
        }
        result[index] = '\0';
    }
    else if (type == 2) {
        for (i = max(x - 4, 0); i <= min(x + 4, BOARD_SIZE - 1); i++) {
            result[index] = board[i][y] + '0';
            ++index;
        }
        result[index] = '\0';
    }
    else if (type == 3) {
        for (i = max({ -4, -x, -y }); i <= min({ 4, BOARD_SIZE - 1 - x, BOARD_SIZE - 1 - y }); i++) {
            result[index] = board[x + i][y + i] + '0';
            ++index;
        }
        result[index] = '\0';
    }
    else if (type == 4) {
        for (i = max({ -4, -x, y + 1 - BOARD_SIZE }); i <= min({ 4, BOARD_SIZE - 1 - x, y }); i++) {
            result[index] = board[x + i][y - i] + '0';
            ++index;
        }
        result[index] = '\0';
    }
    return result;
}

int scoreEvaluation(char* str) {
    // 给某条直线估值
    int sum = 0, temp = 0;
    for (int i = 0; i < 30; i++) {
        temp = Score_Shape[i].score * findSubStrNum(str, Score_Shape[i].shape);
        if (myFlag == 2) temp = -temp;
        if (temp < 0) {
            if (myFlag == 1) temp *= defenseFirstRate;
            else temp *= defenseSecondRate;
        }
        // printf("%s, %s, %d\n", str, Score_Shape[i].shape, temp);
        sum += temp;
    }

    return sum;
}

void getAllScore() {
    // 获取每种直线可能的情况，打表估值
    for (int length = 1; length <= 12; ++length) {
        /*此处本质上是通过模拟一个三进制数来防止多层循环嵌套
         *str为当前正在检查的字符串（也就是该条直线的棋形）
         *endstr为上述字符串的上限值，即222222222222（length==12时）
         *由于在讨论对角线上的棋形时，从棋盘上取出的直线长度可能小于12
         *因此length从1到12都要讨论，共3**13种情况
         *能够在接收到START后的1s内跑完
         */
        vector<int> current;
        char* str = new char[length + 1];
        char* endstr = new char[length + 1];
        for (int i = 0; i < length; ++i) str[i] = '0';
        for (int i = 0; i < length; ++i) endstr[i] = '2';
        endstr[length] = '\0';
        str[length] = '\0';
        for (int i = 1; i <= length; ++i) current.push_back(0);

        while (true) {
            for (int i = 0; i < length; ++i) {
                if (current[i] == 0) str[i] = '0';
                else if (current[i] == 1) str[i] = '1';
                else if (current[i] == 2) str[i] = '2';
            }
            int score = scoreEvaluation(str);

            scoreMap[bkdrHash(str)] = score;

            if (strcmp(str, endstr) == 0) break;

            ++current[length - 1];
            for (int i = length - 1; i > 0; --i) {
                if (current[i] == 3) {
                    current[i] = 0;
                    ++current[i - 1];
                }
            }
        }
        free(str);
        free(endstr);
    }
}

int sumEvaluation() {
    // 给局面估值
    if(DEBUG_MODE) testStart = steady_clock::now();

    int x, y;
    int sum = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        x = i;
        y = 0;
        sum += scoreMap[bkdrHash(boardLine[0][x])];
    }
    for (int i = 0; i < BOARD_SIZE; i++) {
        x = 0;
        y = i;
        sum += scoreMap[bkdrHash(boardLine[1][y])];
    }
    for (int i = 0; i < BOARD_SIZE - 4; i++) {
        x = 0;
        y = i;
        sum += scoreMap[bkdrHash(boardLine[2][x - y + 11])];
    }
    for (int i = 1; i < BOARD_SIZE - 4; i++) {
        x = i;
        y = 0;
        sum += scoreMap[bkdrHash(boardLine[2][x - y + 11])];
    }
    for (int i = 4; i < BOARD_SIZE; i++) {
        x = 0;
        y = i;
        sum += scoreMap[bkdrHash(boardLine[3][x + y])];
    }
    for (int i = 1; i < BOARD_SIZE - 4; i++) {
        x = i;
        y = BOARD_SIZE - 1;
        sum += scoreMap[bkdrHash(boardLine[3][x + y])];
    }

    if(DEBUG_MODE) evaluateTime += getDeltaTime(testStart);

    return sum;
}

inline int deltaEvaluation(Command cmd, int player) {
    // 计算落下cmd，player这颗子后局面估值的变化量
    if(DEBUG_MODE) testStart = steady_clock::now();
    ++evalCount;

    int sum = 0;
    string temp;

    put(cmd, player);
    if (checkGameStatus(cmd)) {
        // 若游戏结束则直接返回MAX/MIN
        if (player == myFlag) sum = INT_MAX;
        else sum = INT_MIN;
        unput(cmd);
    }
    else {
        // 前估值 - 该点为空时所在的四条直线的分别估值 + 该点落下后四条直线的分别估值 = 新估值
        sum += scoreMap[bkdrHash(boardLine[0][cmd.x])];
        sum += scoreMap[bkdrHash(boardLine[1][cmd.y])];
        sum += scoreMap[bkdrHash(boardLine[2][cmd.x - cmd.y + 11])];
        sum += scoreMap[bkdrHash(boardLine[3][cmd.x + cmd.y])];
        unput(cmd);
        sum -= scoreMap[bkdrHash(boardLine[0][cmd.x])];
        sum -= scoreMap[bkdrHash(boardLine[1][cmd.y])];
        sum -= scoreMap[bkdrHash(boardLine[2][cmd.x - cmd.y + 11])];
        sum -= scoreMap[bkdrHash(boardLine[3][cmd.x + cmd.y])];
    }


    if(DEBUG_MODE) evaluateTime += getDeltaTime(testStart);
    return sum;
}

int pointEvaluation(struct Command c, int player) {
    // 粗略的判断某点的价值
    int count5 = 0, count4 = 0, countK4 = 0, count3 = 0, countK3 = 0, count2 = 0;

    // 横向
    int leftSpace = 0, rightSpace = 0, count = 1;
    for (int k = 1, emptyStart = 0; isInBound(c.x - k, c.y) && board[c.x - k][c.y] != 3 - player; ++k) {
        if (!emptyStart) {
            if (board[c.x - k][c.y] == player) ++count;
            else {
                emptyStart = 1;
                ++leftSpace;
            }
        }
        else ++leftSpace;
    }
    for (int k = 1, emptyStart = 0; isInBound(c.x + k, c.y) && board[c.x + k][c.y] != 3 - player; ++k) {
        if (!emptyStart) {
            if (board[c.x + k][c.y] == player) ++count;
            else {
                emptyStart = 1;
                ++rightSpace;
            }
        }
        else ++rightSpace;
    }
    if (count >= 5) ++count5;
    else if (count == 4 && leftSpace && rightSpace) ++count4;
    else if (count == 4 && (leftSpace || rightSpace)) ++countK4;
    else if (count == 3 && leftSpace >= 2 && rightSpace >= 2) ++count3;
    else if (count == 3 && (leftSpace >= 2 || rightSpace >= 2)) ++countK3;
    else if (count == 2 && leftSpace >= 2 && rightSpace >= 2) ++count2;

    // 纵向
    leftSpace = rightSpace = 0;
    count = 1;
    for (int k = 1, emptyStart = 0; isInBound(c.x, c.y - k) && board[c.x][c.y - k] != 3 - player; ++k) {
        if (!emptyStart) {
            if (board[c.x][c.y - k] == player) ++count;
            else {
                emptyStart = 1;
                ++leftSpace;
            }
        }
        else ++leftSpace;
    }
    for (int k = 1, emptyStart = 0; isInBound(c.x, c.y + k) && board[c.x][c.y + k] != 3 - player; ++k) {
        if (!emptyStart) {
            if (board[c.x][c.y + k] == player) ++count;
            else {
                emptyStart = 1;
                ++rightSpace;
            }
        }
        else ++rightSpace;
    }
    if (count >= 5) ++count5;
    else if (count == 4 && leftSpace && rightSpace) ++count4;
    else if (count == 4 && (leftSpace || rightSpace)) ++countK4;
    else if (count == 3 && leftSpace >= 2 && rightSpace >= 2) ++count3;
    else if (count == 3 && (leftSpace >= 2 || rightSpace >= 2)) ++countK3;
    else if (count == 2 && leftSpace >= 2 && rightSpace >= 2) ++count2;

    // 主对角线
    leftSpace = rightSpace = 0;
    count = 1;
    for (int k = 1, emptyStart = 0; isInBound(c.x - k, c.y - k) && board[c.x - k][c.y - k] != 3 - player; ++k) {
        if (!emptyStart) {
            if (board[c.x - k][c.y - k] == player) ++count;
            else {
                emptyStart = 1;
                ++leftSpace;
            }
        }
        else ++leftSpace;
    }
    for (int k = 1, emptyStart = 0; isInBound(c.x + k, c.y + k) && board[c.x + k][c.y + k] != 3 - player; ++k) {
        if (!emptyStart) {
            if (board[c.x + k][c.y + k] == player) ++count;
            else {
                emptyStart = 1;
                ++rightSpace;
            }
        }
        else ++rightSpace;
    }
    if (count >= 5) ++count5;
    else if (count == 4 && leftSpace && rightSpace) ++count4;
    else if (count == 4 && (leftSpace || rightSpace)) ++countK4;
    else if (count == 3 && leftSpace >= 2 && rightSpace >= 2) ++count3;
    else if (count == 3 && (leftSpace >= 2 || rightSpace >= 2)) ++countK3;
    else if (count == 2 && leftSpace >= 2 && rightSpace >= 2) ++count2;

    // 副对角线
    leftSpace = rightSpace = 0;
    count = 1;
    for (int k = 1, emptyStart = 0; isInBound(c.x - k, c.y + k) && board[c.x - k][c.y + k] != 3 - player; ++k) {
        if (!emptyStart) {
            if (board[c.x - k][c.y + k] == player) ++count;
            else {
                emptyStart = 1;
                ++leftSpace;
            }
        }
        else ++leftSpace;
    }
    for (int k = 1, emptyStart = 0; isInBound(c.x + k, c.y - k) && board[c.x + k][c.y - k] != 3 - player; ++k) {
        if (!emptyStart) {
            if (board[c.x + k][c.y - k] == player) ++count;
            else {
                emptyStart = 1;
                ++rightSpace;
            }
        }
        else ++rightSpace;
    }
    if (count >= 5) ++count5;
    else if (count == 4 && leftSpace && rightSpace) ++count4;
    else if (count == 4 && (leftSpace || rightSpace)) ++countK4;
    else if (count == 3 && leftSpace >= 2 && rightSpace >= 2) ++count3;
    else if (count == 3 && (leftSpace >= 2 || rightSpace >= 2)) ++countK3;
    else if (count == 2 && leftSpace >= 2 && rightSpace >= 2) ++count2;

    // 总结评分
    if (count5) return INT_MAX;
    if (count4) return 99999999;
    return countK4 * 10000 + count3 * 1000 + countK3 * 200 + count2 * 200;
}

bool isK4KillingLine(char* str, int player) {
    // 是否是卡四
    for (int i = 0; i < 5; i++) {
        if (findSubStr(str, k4Patterns[player - 1][i])) return true;
    }
    return false;
}

bool isH3KillingLine(char* str, int player) {
    // 是否是活三
    for (int i = 0; i < 4; i++) {
        if (findSubStr(str, h3Patterns[player - 1][i])) return true;
    }
    return false;
}

bool isKillingPoint(int player, Command c) {
    // 该点是否有活三/卡四
    for (int i = 1; i <= 4; ++i) {
        char* rangedLine = rangedTransform(i, c.x, c.y);
        if (isK4KillingLine(rangedLine, player) || isH3KillingLine(rangedLine, player)) {
            return true;
        }
    }
    return false;
}

vector<CommandVectorNode>* getNextMovesVector(int player) {
    // 获取下一步可用的位置
    steady_clock::time_point tempStart = steady_clock::now();

    int tempBoardCheck[BOARD_SIZE][BOARD_SIZE];
    memset(tempBoardCheck, FALSE, sizeof(tempBoardCheck));
    vector<CommandVectorNode>* cVector = new vector<CommandVectorNode>;

    // 对于棋盘上的每个点，寻找与其同一条直线上range范围内的空位置
    for (int p = 0; p < onBoardCommands.size(); ++p) {
        Command command = onBoardCommands[p];
        int i = command.x, j = command.y;
        for (int k = -range; k <= range; ++k) {
            if (k == 0) {
                continue;
            }
            if (isInBound(i + k, j) && !tempBoardCheck[i + k][j] && board[i + k][j] == EMPTY) {  // 竖向
                tempBoardCheck[i + k][j] = TRUE;
                CommandVectorNode c;
                c.com.x = i + k;
                c.com.y = j;
                c.pointValue = deltaEvaluation(c.com, player);
                c.pointValue -= deltaEvaluation(c.com, 3 - player);
                cVector->push_back(c);
            }
            if (isInBound(i, j + k) && !tempBoardCheck[i][j + k] && board[i][j + k] == EMPTY) {  // 横向
                tempBoardCheck[i][j + k] = TRUE;
                CommandVectorNode c;
                c.com.x = i;
                c.com.y = j + k;
                c.pointValue = deltaEvaluation(c.com, player);
                c.pointValue -= deltaEvaluation(c.com, 3 - player);
                cVector->push_back(c);
            }
            if (isInBound(i + k, j + k) && !tempBoardCheck[i + k][j + k] && board[i + k][j + k] == EMPTY) {  // 主对角线
                tempBoardCheck[i + k][j + k] = TRUE;
                CommandVectorNode c;
                c.com.x = i + k;
                c.com.y = j + k;
                c.pointValue = deltaEvaluation(c.com, player);
                c.pointValue -= deltaEvaluation(c.com, 3 - player);
                cVector->push_back(c);
            }
            if (isInBound(i + k, j - k) && !tempBoardCheck[i + k][j - k] && board[i + k][j - k] == EMPTY) {  // 副对角线
                tempBoardCheck[i + k][j - k] = TRUE;
                CommandVectorNode c;
                c.com.x = i + k;
                c.com.y = j - k;
                c.pointValue = deltaEvaluation(c.com, player);
                c.pointValue -= deltaEvaluation(c.com, 3 - player);
                cVector->push_back(c);
            }
        }
    }

    vector<CommandVectorNode>* newCVector = new vector<CommandVectorNode>;
    for (CommandVectorNode node : *cVector) {
        if (pointEvaluation(node.com, player) == INT_MAX) {  // 若我方有连五，则直接返回
            newCVector->push_back(node);
            free(cVector);
            if(DEBUG_MODE) getMovesTime += getDeltaTime(tempStart);
            cuttedMoves = true;
            return newCVector;
        }
        int enemyValue = pointEvaluation(node.com, 3 - player);
        if (enemyValue == INT_MAX) {  // 对方可以连五，也直接返回
            newCVector->push_back(node);
        }
    }

    if (!newCVector->empty()) {  // 如果没有必杀棋且对方可以连五，就只能防守
        free(cVector);
        if (DEBUG_MODE) getMovesTime += getDeltaTime(tempStart);
        cuttedMoves = true;
        return newCVector;
    }

    free(newCVector);
    if (player == enemyFlag) {
        // 当player==enemyFlag时整个估值会是相反数，需要再取负值
        for (int i = 0; i < cVector->size(); ++i) {
            if ((*cVector)[i].pointValue == INT_MIN) (*cVector)[i].pointValue = INT_MAX;
            else (*cVector)[i].pointValue *= -1;
        }
    }
    sort(cVector->begin(), cVector->end(), cVectorCmp);
    cuttedMoves = false;

    vector<CommandVectorNode>* returnCVector = new vector<CommandVectorNode>;
    for (int i = 0; i < cVector->size(); ++i) {  // 去掉估值不为正的点
        if ((*cVector)[i].pointValue > 0) continue;
        returnCVector->assign(cVector->begin(), cVector->begin() + i);
        break;
    }
    free(cVector);
    if (DEBUG_MODE) getMovesTime += getDeltaTime(tempStart);
    return returnCVector;
}

bool vcfAlphaBeta(int depth, int maximizingPlayer, int maximizingTarget) {
    // 计算VCF
    if (!DEBUG_MODE) {
        if (getDeltaTime(globalStartTime) > 1.8) {  // 在时间不够时迅速跳出
            timeLimitBreak = true;
            if (maximizingPlayer) return false;
            else return true;
        }
    }

    if (maximizingPlayer == maximizingTarget) {  // Maximizing player
        auto cVector = getNextMovesVector(maximizingPlayer);
        for (CommandVectorNode node : *cVector) {
            if (pointEvaluation(node.com, maximizingPlayer) == INT_MAX) return true;  // 之后可能要写个更快的函数替代pointEvaluation
            if (depth == VCF_MAX_DEPTH) continue;  // 如果最大深度，就只检测能不能杀，不进一步vcf
            Command c = node.com;

            // Try placing the piece
            put(c, maximizingPlayer);
            if (!isK4KillingLine(rangedTransform(1, c.x, c.y), maximizingTarget) &&
                !isK4KillingLine(rangedTransform(2, c.x, c.y), maximizingTarget) &&
                !isK4KillingLine(rangedTransform(3, c.x, c.y), maximizingTarget) &&
                !isK4KillingLine(rangedTransform(4, c.x, c.y), maximizingTarget)) {  // 如果没有卡四就跳过这个点
                unput(c);
                continue;
            };

            bool eval = vcfAlphaBeta(depth + 1, 3 - maximizingPlayer, maximizingTarget);
            unput(c);
            if (eval) return true;  // max节点，一个杀的掉就是杀的掉
        }

        delete cVector;
        return false;  // 全都杀不掉就是杀不掉
    }
    else { // Minimizing player
        auto cVector = getNextMovesVector(3 - maximizingTarget);
        for (CommandVectorNode node : *cVector) {
            if (pointEvaluation(node.com, 3 - maximizingTarget) == INT_MAX) return false;  // 之后可能要写个更快的函数替代pointEvaluation
            if (depth == VCF_MAX_DEPTH) continue;  // 如果最大深度，就只检测能不能杀，不进一步vcf
            if (pointEvaluation(node.com, maximizingTarget) == INT_MAX) {  // 堵住对方的杀棋，或者自己必杀
                put(node.com, 3 - maximizingTarget);
                bool eval = vcfAlphaBeta(depth + 1, 3 - maximizingPlayer, maximizingTarget);
                unput(node.com);
                if (!eval) return false;  // min节点，只要有一个杀不掉就是杀不掉
            }
        }

        delete cVector;
        if (depth == VCF_MAX_DEPTH) return false;  // 无法算完的情况返回杀不掉
        return true;  // 全都杀的掉就是杀的掉
    }
}

bool vctAlphaBeta(int depth, int maximizingPlayer, int maximizingTarget, Command lastMove) {
    // 计算VCT
    if (TIME_LIMIT) {
        if (getDeltaTime(globalStartTime) > 1.3) {  // 在时间不够时迅速跳出
            timeLimitBreak = true;
            return false;
        }
    }

    if (maximizingPlayer == maximizingTarget) {  // Maximizing player
        auto cVector = getNextMovesVector(maximizingPlayer);
        for (CommandVectorNode node : *cVector) {
            if (TIME_LIMIT) {
                if (getDeltaTime(globalStartTime) > 1.8) {  // 在时间不够时迅速跳出
                    timeLimitBreak = true;
                    return false;
                }
            }

            if (pointEvaluation(node.com, maximizingPlayer) == INT_MAX) {
                delete cVector;
                return true;
            }
            if (depth == VCT_MAX_DEPTH) continue;  // 如果最大深度，就只检测能不能杀，不进一步vct

            Command c = node.com;
            // 尝试落子
            put(c, maximizingPlayer);
            bool isKill = false;
            for (int i = 1; i <= 4; ++i) {
                char* rangedLine = rangedTransform(i, c.x, c.y);
                if (isK4KillingLine(rangedLine, maximizingTarget) || isH3KillingLine(rangedLine, maximizingTarget)) {
                    isKill = true;
                    break;
                }
            }
            if (!isKill) {  // 没找到活三/卡四就跳过
                unput(c);
                continue;
            }

            bool eval = vctAlphaBeta(depth + 1, 3 - maximizingPlayer, maximizingTarget, c);

            unput(c);
            if (eval) {
                delete cVector;
                return true;  // max节点，一个杀的掉就是杀的掉
            }
        }

        delete cVector;
        return false;  // 全都杀不掉就是杀不掉
    }
    else { // Minimizing player
        auto cVector = getNextMovesVector(3 - maximizingTarget);
        for (CommandVectorNode node : *cVector) {
            if (TIME_LIMIT) {
                if (getDeltaTime(globalStartTime) > 1.3) {  // 在时间不够时迅速跳出
                    timeLimitBreak = true;
                    return false;
                }
            }

            if (pointEvaluation(node.com, 3 - maximizingTarget) == INT_MAX) {
                delete cVector;
                return false;
            }
            if (depth == VCT_MAX_DEPTH) continue;  // 如果最大深度，就只检测能不能杀，不进一步vcf

            Command c = node.com;
            bool isKill = false;
            for (int i = 1; i <= 4; ++i) {
                // 如果这个棋子能防守住对方的棋子，AKA对方原本能杀，放了后不能杀
                char* lastMoveLine = rangedTransform(i, lastMove.x, lastMove.y);
                if (isK4KillingLine(lastMoveLine, maximizingTarget) || isH3KillingLine(lastMoveLine, maximizingTarget)) {
                    put(node.com, 3 - maximizingTarget);
                    lastMoveLine = rangedTransform(i, lastMove.x, lastMove.y);
                    if (!isK4KillingLine(lastMoveLine, maximizingTarget) && !isH3KillingLine(lastMoveLine, maximizingTarget)) {
                        isKill = true;
                        break;
                    }
                    unput(c);
                }

                // 如果这个棋子己方能杀
                put(node.com, 3 - maximizingTarget);
                char* rangedLine = rangedTransform(i, c.x, c.y);
                if (pointEvaluation(node.com, 3 - maximizingTarget) == INT_MAX ||
                    isK4KillingLine(rangedLine, 3 - maximizingTarget)) {
                    isKill = true;
                    unput(c);
                    put(c, 3 - maximizingTarget);
                    break;
                }
                unput(c);
            }
            if (!isKill) {  // 没找到活三/卡四就跳过
                continue;
            }

            bool eval = vctAlphaBeta(depth + 1, 3 - maximizingPlayer, maximizingTarget, c);
            unput(node.com);

            if (!eval) {
                delete cVector;
                return false;  // min节点，只要有一个杀不掉就是杀不掉
            }
        }

        delete cVector;
        if (depth == VCT_MAX_DEPTH) return false;  // 无法算完的情况返回杀不掉
        return true;  // 全都杀的掉就是杀的掉
    }
}

int alphaBeta(int depth, int alpha, int beta, int maximizingPlayer, vector<CommandVectorNode>* lastMoves = nullptr, Command lastMove = Command()) {
    // alphabeta剪枝
    if (TIME_LIMIT) {
        if (getDeltaTime(globalStartTime) > 1.3) {  // 在时间不够时迅速跳出
            timeLimitBreak = true;
            if (maximizingPlayer == myFlag) return INT_MAX;    // 该结果返回给上层min节点，因此为MAX
            return INT_MIN;
        }
    }

    if (zobrist.isExist() && zobrist.score[zobrist.value].second >= depth) {  // 查表
        ++zobristCount;
        return zobrist.score[zobrist.value].first;
    }
    ++alphaBetaCount;

    int eval, oldEvaluationValue = evaluationValue;
    if (maximizingPlayer == myFlag) {  // Maximizing player
        // vector<CommandVectorNode>* cVector;
        // if (lastMoves == nullptr || cuttedMoves) cVector = getNextMovesVector(maximizingPlayer);
        // else cVector = getDeltaMovesVector(maximizingPlayer, lastMove, *lastMoves);
        auto cVector = getNextMovesVector(maximizingPlayer);
        for (CommandVectorNode node : *cVector) {
            if (TIME_LIMIT) {
                if (getDeltaTime(globalStartTime) > 1.2) {  // 在时间不够时迅速跳出
                    timeLimitBreak = true;
                    return INT_MAX;    // 该结果返回给上层min节点，因此为MAX
                }
            }

            struct Command c = node.com;

            if (depth == MAX_DEPTH - 1) {
                put(c, maximizingPlayer);
                if (zobrist.isExist() && zobrist.score[zobrist.value].second >= depth) {
                    eval = zobrist.score[zobrist.value].first;
                    ++zobristCount;
                    unput(c);
                }
                else {
                	unput(c);
                    eval = deltaEvaluation(c, myFlag);
                    if (eval != INT_MAX && eval != INT_MIN) eval += evaluationValue;

                    zobrist.add(c, maximizingPlayer);
                    zobrist.record(eval, depth);
                    zobrist.add(c, maximizingPlayer);
                }
            }
            else {
                // 尝试落子
                int temp = deltaEvaluation(c, myFlag);
                if (temp == INT_MAX) {
                    alpha = INT_MAX;
                    break;
                }
                else {
                    evaluationValue = oldEvaluationValue + temp;
                    put(c, myFlag);  // 2023.11.9，有我好找的

                    // 递归寻找下一步
                    if (checkGameStatus(c)) eval = INT_MAX;
                    else eval = alphaBeta(depth + 1, alpha, beta, 3 - maximizingPlayer, cVector, c);

                    // 提子
                    unput(c);

                    if (TIME_LIMIT && timeLimitBreak) {
                        return INT_MAX;    // 该结果返回给上层min节点，因此为MAX
                    }
                }
            }

            zobrist.add(c, maximizingPlayer);
            zobrist.record(eval, depth);
            zobrist.add(c, maximizingPlayer);

            alpha = alpha > eval ? alpha : eval;
            if (beta <= alpha) {  // Beta pruning
                delete cVector;
                zobrist.record(eval, depth);
                return beta;
            }
        }

        delete cVector;
        zobrist.record(eval, depth);
        return alpha;
    }
    else { // Minimizing player
        auto cVector = getNextMovesVector(maximizingPlayer);
        for (CommandVectorNode node : *cVector) {
            if (TIME_LIMIT) {
                if (getDeltaTime(globalStartTime) > 1.2) {  // 在时间不够时迅速跳出
                    timeLimitBreak = true;
                    return INT_MIN;  // 该结果返回给上层max节点，因此为MIN
                }
            }

            Command c = node.com;

            if (depth == MAX_DEPTH - 1) {
                put(c, maximizingPlayer);
                if (zobrist.isExist() && zobrist.score[zobrist.value].second >= depth) {
                    eval = zobrist.score[zobrist.value].first;
                    ++zobristCount;
                    unput(c);
                }
                else {
                    if (myFlag == 2) {  // if myflag == 2
                        if (isKillingPoint(maximizingPlayer, c)) {
                            // bool canKill = vctAlphaBeta(4, 3 - maximizingPlayer, maximizingPlayer, c);
                            bool canKill = vcfAlphaBeta(0, 3 - maximizingPlayer, maximizingPlayer);
                            unput(c);
                            if (canKill) eval = INT_MIN;
                            else {
                                eval = deltaEvaluation(c, maximizingPlayer);
                                if (eval != INT_MIN && eval != INT_MAX) eval += evaluationValue;
                            }
                        }
                        else {
                            unput(c);
                            eval = deltaEvaluation(c, maximizingPlayer);
                            if (eval != INT_MIN && eval != INT_MAX) eval += evaluationValue;
                        }
                    }
                    else {
                        unput(c);
                        eval = deltaEvaluation(c, maximizingPlayer);  // 2023.11.10 MD又是什么狗屎
                        if (eval != INT_MIN && eval != INT_MAX) eval += evaluationValue;
                    }

                    zobrist.add(c, maximizingPlayer);
                    zobrist.record(eval, depth);
                    zobrist.add(c, maximizingPlayer);
                }
            }
            else {
                // 尝试落子
                int temp = deltaEvaluation(c, enemyFlag);
                if (temp == INT_MIN) {
                    beta = INT_MIN;
                    break;
                }
                else {
                    evaluationValue = oldEvaluationValue + temp;
                    put(c, enemyFlag);

                    // 递归寻找下一步
                    if (checkGameStatus(c)) eval = INT_MIN;
                    else eval = alphaBeta(depth + 1, alpha, beta, 3 - maximizingPlayer, cVector, c);

                    // 提子
                    unput(c);

                    if (TIME_LIMIT && timeLimitBreak) {
                        return INT_MIN;  // 该结果返回给上层max节点，因此为MINa
                    }
                }
            }

            zobrist.add(c, maximizingPlayer);
            zobrist.record(eval, depth);
            zobrist.add(c, maximizingPlayer);

            beta = beta < eval ? beta : eval;
            if (beta <= alpha) {  // Alpha剪枝
                delete cVector;
                zobrist.record(eval, depth);
                return alpha;
            }
        }

        delete cVector;
        zobrist.record(eval, depth);
        return beta;
    }
}

Command findBest(int alpha, int beta, int maximizingPlayer) {
    /*实现流程：
     *1.寻找开局库中是否有当前局面，有就直接输出
     *2.寻找我方VCF胜法，有就直接输出
     *3.寻找我方VCT胜法，有就直接输出
     *4.若己方执黑，则搜索对方VCF; 若己方执白，则搜索对方VCT；后续AlphaBeta仅搜索对方的必胜子
     *5.AlphaBeta运算
     */

    // 寻找落子位置
    if (opening.find(zobrist.value) != opening.end()) {
        pair<int, int> p = opening[zobrist.value];
        Command maxCommand;
        maxCommand.x = p.first;
        maxCommand.y = p.second;
        if (DEBUG_MODE) printf("Found Opening :%d\n", zobrist.value);
        return maxCommand;
    }

    int depth = 0;
    int maxEval = INT_MIN;
    int maxDepth = 0;
    cuttedMoves = false;
    double averageTime;
    double remainTime = 2.0;
    globalStartTime = steady_clock::now();
    vector<CommandVectorNode>* cVector = getNextMovesVector(maximizingPlayer);
    Command maxCommand = (*cVector)[0].com;
    evaluationValue = sumEvaluation();
    timeLimitBreak = false;

    // 先过一遍VCF
    for (CommandVectorNode node : *cVector) {
        if (pointEvaluation(node.com, maximizingPlayer) == INT_MAX) {
            delete cVector;
            return node.com;
        }
        put(node.com, maximizingPlayer);
        if (!isK4KillingLine(rangedTransform(1, node.com.x, node.com.y), maximizingPlayer) &&
            !isK4KillingLine(rangedTransform(2, node.com.x, node.com.y), maximizingPlayer) &&
            !isK4KillingLine(rangedTransform(3, node.com.x, node.com.y), maximizingPlayer) &&
            !isK4KillingLine(rangedTransform(4, node.com.x, node.com.y), maximizingPlayer)) {
            unput(node.com);
            continue;
        }  // 如果没有卡四就跳过这个点

        bool canKill = vcfAlphaBeta(1, 3 - maximizingPlayer, maximizingPlayer);
        unput(node.com);

        if (canKill) {
            if (DEBUG_MODE) {
                cout << "Found VCF!" << endl;
                printf("%lf\n", getDeltaTime(globalStartTime));
            }
            return node.com;
        }
    }

    // 再过一遍自己的VCT
    for (CommandVectorNode node : *cVector) {
        Command c = node.com;
        // Try placing the piece
        put(c, maximizingPlayer);
        bool isKill = false;
        for (int i = 1; i <= 4; ++i) {
            char* rangedLine = rangedTransform(i, c.x, c.y);
            if (isK4KillingLine(rangedLine, maximizingPlayer) || isH3KillingLine(rangedLine, maximizingPlayer)) {
                isKill = true;
                break;
            }
        }
        if (!isKill) {  // 没找到活三/卡四就跳过
            unput(c);
            continue;
        }

        bool canKill = vctAlphaBeta(1, 3 - maximizingPlayer, maximizingPlayer, c);
        unput(c);

        if (canKill) {
            if (DEBUG_MODE) {
                cout << "Found VCT!" << endl;
                printf("%lf\n", getDeltaTime(globalStartTime));
            }
            return node.com;
        }
    }

    if (myFlag == 1) {
        // 帮对手找一遍VCF
        vector<CommandVectorNode>* newVector = new vector<CommandVectorNode>;  // 多个杀点选其一防
        for (CommandVectorNode node : *cVector) {
            if (pointEvaluation(node.com, 3 - maximizingPlayer) == INT_MAX) {
                delete cVector;
                return node.com;
            }
            Command c = node.com;
            // Try placing the piece
            put(c, maximizingPlayer);

            bool canKill = vcfAlphaBeta(0, 3 - maximizingPlayer, 3 - maximizingPlayer);  // 我方下c时对方是否能杀
            unput(c);

            if (!canKill) {  // 我方下c时对方不能杀
                CommandVectorNode newNode;
                newNode.com = c;
                newNode.pointValue = deltaEvaluation(c, maximizingPlayer) - deltaEvaluation(c, 3 - maximizingPlayer);
                newVector->push_back(newNode);
            }
        }
        if (!newVector->empty()) {  // 我方有杀不死的子
            cVector->clear();
            for (auto i : *newVector) {
                cVector->emplace_back(i);
            }
            sort(cVector->begin(), cVector->end(), cVectorCmp);
        }
        free(newVector);
    }
    if (myFlag == 2) {
        // 帮对手找一遍VCT
        vector<CommandVectorNode>* newVector = new vector<CommandVectorNode>;  // 多个杀点选其一防
        for (CommandVectorNode node : *cVector) {
            if (pointEvaluation(node.com, 3 - maximizingPlayer) == INT_MAX) {
                delete cVector;
                return node.com;
            }
            Command c = node.com;
            put(c, maximizingPlayer);

            bool canKill = vctAlphaBeta(0, 3 - maximizingPlayer, 3 - maximizingPlayer, c);  // 我方下c时对方是否能杀
            unput(c);

            if (!canKill) {  // 我方下c时对方不能杀
                CommandVectorNode newNode;
                newNode.com = c;
                newNode.pointValue = deltaEvaluation(c, maximizingPlayer) - deltaEvaluation(c, 3 - maximizingPlayer);
                newVector->push_back(newNode);
            }
        }
        if (!newVector->empty()) {  // 我方有杀不死的子
            cVector->clear();
            for (auto i : *newVector) {
                cVector->emplace_back(i);
            }
            sort(cVector->begin(), cVector->end(), cVectorCmp);
        }
        free(newVector);
    }


    int eval;
    const int oldEvaluationValue = evaluationValue;
    if (onBoardCommands.size() > 115) maxDepth = 3;
    else maxDepth = 5;
    for (MAX_DEPTH = 3; MAX_DEPTH <= maxDepth; MAX_DEPTH += 2) {
        startTime = steady_clock::now();
        zobrist.score.clear();
        for (int i = 0; i < cVector->size(); ++i) {
            Command c = (*cVector)[i].com;

            // 尝试落子
            evaluationValue = oldEvaluationValue + deltaEvaluation(c, myFlag);
            put(c, myFlag);

            // 递归寻找下一步
            if (checkGameStatus(c) == 1) eval = INT_MAX;
            else eval = alphaBeta(depth + 1, alpha, beta, 3 - maximizingPlayer);

            unput(c);

            if (eval > maxEval && MAX_DEPTH >= maxDepth) {
                maxEval = eval;
                maxCommand = c;
                maxDepth = MAX_DEPTH;
            }
            if (maxEval == INT_MAX) break;

            (*cVector)[i].pointValue = eval;

            averageTime = getDeltaTime(startTime) / (i + 1);
            if (TIME_LIMIT) {
                if (remainTime - getDeltaTime(startTime) < 5 * averageTime) {
                    timeLimitBreak = true;
                    break;
                }
                if (timeLimitBreak) break;
            }
        }
        if (TIME_LIMIT) {
            if (timeLimitBreak || remainTime < 20 * averageTime) {
                if (DEBUG_MODE) cout << "TimeLimitBreak!!!!" << endl;
                break;
            }
        }
        remainTime -= getDeltaTime(startTime);
        sort(cVector->begin(), cVector->end(), cVectorCmp);
    }

    delete cVector;
    if (DEBUG_MODE) printf("%d %lf\n", maxEval, getDeltaTime(startTime));
    return maxCommand;
}

void turn() {
    if(DEBUG_MODE) {
        evaluateTime = checkBoardTime = alphaBetaCount = zobristCount = getMovesTime = evalCount = 0;
    }
	Command bestCommand = findBest(INT_MIN + 1, INT_MAX, myFlag);
    put(bestCommand, myFlag);
    if (DEBUG_MODE) {
        printf("AlphaBeta Count: %d\n", alphaBetaCount);
        printf("Zobrist Count: %d\n", zobristCount);
        printf("Total Time: %lf\n", getDeltaTime(globalStartTime));
        printf("Eval Time: %lf\n", evaluateTime);
        printf("Eval Count: %d\n", evalCount);
        printf("Check Board Time: %lf\n", checkBoardTime);
        printf("Get Moves Time: %lf\n", getMovesTime);
    }
    printf("%d %d", bestCommand.x, bestCommand.y);
    printf("\n");
    fflush(stdout);
    if (DEBUG_MODE) printBoard();
}

void end(int x) {
    exit(0);
}

void loop() {
    char tag[10] = { 0 };
    Command command;
    command.x = command.y = 0;
    int status;
    while (TRUE) {
        memset(tag, 0, sizeof(tag));
        scanf("%s", tag);
        if (strcmp(tag, START) == 0) {
            scanf("%d", &myFlag);
            start(myFlag);
            printf("OK\n");
            fflush(stdout);
        }
        else if (strcmp(tag, PLACE) == 0) {
            scanf("%d %d", &command.x, &command.y);
            place(command);
            enemyLastMove = command;
        }
        else if (strcmp(tag, TURN) == 0) {
            turn();
        }
        else if (strcmp(tag, END) == 0) {
            scanf("%d", &status);
            end(status);
        }
        else if (strcmp(tag, "EVALUATE") == 0) {
            cout << sumEvaluation() << endl;
        }
    }
}

int main() {
    /*用到的技术/优化方法：
     *AlphaBeta剪枝
     *VCF/VCT计算（基于AlphaBeta剪枝）
     *迭代加深
     *启发式搜索
     *Zobrist置换表
     *开局库
     *动态更新全局评估值（基于点）
     *采用哈希表快速估值
     */
    if(true) {
        // 打个小表
        opening[0] = pair<int, int>(7, 7);
        opening[1967487797] = pair<int, int>(8, 7);
        opening[-877794435] = pair<int, int>(9, 7);
        opening[-800491373] = pair<int, int>(8, 5);
        opening[-110401401] = pair<int, int>(10, 7);
        opening[727324694] = pair<int, int>(9, 6);
        opening[-1602134282] = pair<int, int>(10, 5);
        opening[-1754554813] = pair<int, int>(9, 5);
        opening[-1221341534] = pair<int, int>(8, 6);
        opening[-1933288920] = pair<int, int>(7, 5);
        opening[568105453] = pair<int, int>(9, 5);
        opening[1439352174] = pair<int, int>(7, 6);
        opening[-1791523952] = pair<int, int>(8, 6);
        opening[545747580] = pair<int, int>(9, 7);
        opening[1794034924] = pair<int, int>(9, 4);
        opening[-802638734] = pair<int, int>(7, 8);
        opening[1426658259] = pair<int, int>(9, 8);
        opening[1111603994] = pair<int, int>(9, 6);
        opening[1901943113] = pair<int, int>(10, 7);
        opening[-648186302] = pair<int, int>(7, 10);
        opening[1165013580] = pair<int, int>(7, 5);
        opening[809389399] = pair<int, int>(8, 6);
        opening[511691923] = pair<int, int>(9, 8);
        opening[810634981] = pair<int, int>(10, 7);
        opening[1790412948] = pair<int, int>(9, 5);
        opening[-1490277356] = pair<int, int>(7, 6);
        opening[-1214883018] = pair<int, int>(7, 5);
        opening[-1941581476] = pair<int, int>(7, 4);
        opening[-1187457600] = pair<int, int>(8, 5);
        opening[-122320765] = pair<int, int>(8, 3);
        opening[1459641676] = pair<int, int>(8, 6);
        opening[877971926] = pair<int, int>(6, 8);
        opening[590419043] = pair<int, int>(7, 5);
        opening[-1909860442] = pair<int, int>(8, 5);
        opening[998527651] = pair<int, int>(8, 4);
        opening[-391052271] = pair<int, int>(8, 7);
        opening[-520318838] = pair<int, int>(7, 6);
        opening[-811300123] = pair<int, int>(8, 8);
        opening[1617165946] = pair<int, int>(5, 4);
        opening[1610335516] = pair<int, int>(7, 8);
        opening[-491737952] = pair<int, int>(7, 4);
        opening[1344996933] = pair<int, int>(7, 5);
        opening[-46938752] = pair<int, int>(8, 7);
        opening[-1048909501] = pair<int, int>(5, 4);
        opening[-932802990] = pair<int, int>(8, 7);
        opening[-806934062] = pair<int, int>(3, 7);
        opening[1574405207] = pair<int, int>(8, 9);
        opening[-1662397665] = pair<int, int>(8, 2);
        opening[-1648409195] = pair<int, int>(7, 9);
        opening[1491751998] = pair<int, int>(6, 9);
        opening[570178005] = pair<int, int>(7, 3);
        opening[-1554236940] = pair<int, int>(9, 8);
        opening[-1921860734] = pair<int, int>(9, 7);
        opening[-1768544148] = pair<int, int>(9, 6);
        opening[777140744] = pair<int, int>(7, 4);
        opening[-954530542] = pair<int, int>(9, 6);
        opening[2145326966] = pair<int, int>(7, 8);
        opening[-146818345] = pair<int, int>(6, 7);
        opening[-1482174065] = pair<int, int>(4, 6);
        opening[-1087098938] = pair<int, int>(6, 7);
        opening[-1899048183] = pair<int, int>(7, 8);
        opening[305897589] = pair<int, int>(7, 6);
        opening[-803487157] = pair<int, int>(7, 6);
        opening[1414864900] = pair<int, int>(6, 8);
        opening[-2124393033] = pair<int, int>(4, 7);
        opening[-52267966] = pair<int, int>(7, 4);
        opening[1806272090] = pair<int, int>(8, 6);
        opening[150803136] = pair<int, int>(3, 8);
        opening[1475237062] = pair<int, int>(4, 8);
        opening[426770628] = pair<int, int>(4, 9);
        opening[-1927843013] = pair<int, int>(4, 9);
        opening[-1908227035] = pair<int, int>(4, 9);
        opening[-1806878443] = pair<int, int>(4, 7);
        opening[1608726504] = pair<int, int>(4, 6);
        opening[-1482575307] = pair<int, int>(4, 8);
        opening[647239695] = pair<int, int>(7, 8);
        opening[4204129] = pair<int, int>(4, 8);
        opening[-1497531937] = pair<int, int>(3, 5);
        opening[254597723] = pair<int, int>(7, 9);
        opening[1464572075] = pair<int, int>(4, 8);
        opening[-239679723] = pair<int, int>(7, 8);
        opening[-1589295270] = pair<int, int>(5, 8);
        opening[-334499659] = pair<int, int>(3, 8);
        opening[-1289963853] = pair<int, int>(4, 8);
        opening[706134202] = pair<int, int>(7, 6);
        opening[1212821171] = pair<int, int>(4, 3);
        opening[-2055652099] = pair<int, int>(4, 2);
        opening[-617379266] = pair<int, int>(6, 3);
        opening[-976591948] = pair<int, int>(4, 1);
        opening[1996146709] = pair<int, int>(5, 2);
        opening[577191575] = pair<int, int>(6, 1);
        opening[64458044] = pair<int, int>(6, 2);
        opening[1017345801] = pair<int, int>(5, 3);
        opening[795285345] = pair<int, int>(6, 4);
        opening[195856145] = pair<int, int>(6, 2);
        opening[-2078514788] = pair<int, int>(5, 4);
        opening[341140222] = pair<int, int>(5, 3);
        opening[-328529396] = pair<int, int>(4, 2);
        opening[-567192832] = pair<int, int>(7, 2);
        opening[-956269714] = pair<int, int>(3, 4);
        opening[-1624858944] = pair<int, int>(3, 2);
        opening[178827587] = pair<int, int>(5, 2);
        opening[670459552] = pair<int, int>(4, 1);
        opening[320516393] = pair<int, int>(1, 4);
        opening[-549526720] = pair<int, int>(6, 4);
        opening[-198842299] = pair<int, int>(5, 3);
        opening[1246630634] = pair<int, int>(3, 2);
        opening[773000216] = pair<int, int>(4, 1);
        opening[-1195283259] = pair<int, int>(6, 2);
        opening[639817416] = pair<int, int>(5, 4);
        opening[2097173524] = pair<int, int>(6, 4);
        opening[175160677] = pair<int, int>(7, 4);
        opening[1820707559] = pair<int, int>(6, 3);
        opening[1701947253] = pair<int, int>(8, 3);
        opening[-1605403554] = pair<int, int>(5, 3);
        opening[-1197445320] = pair<int, int>(3, 5);
        opening[1064793264] = pair<int, int>(6, 4);
        opening[465364160] = pair<int, int>(6, 3);
        opening[1626756070] = pair<int, int>(7, 3);
        opening[-2135306291] = pair<int, int>(4, 3);
        opening[-1604942007] = pair<int, int>(5, 4);
        opening[306743634] = pair<int, int>(3, 3);
        opening[1315609197] = pair<int, int>(7, 6);
        opening[-231200231] = pair<int, int>(3, 4);
        opening[1483805868] = pair<int, int>(7, 4);
        opening[-1841273223] = pair<int, int>(6, 4);
        opening[-1232341495] = pair<int, int>(4, 3);
        opening[1743947136] = pair<int, int>(7, 6);
        opening[586120319] = pair<int, int>(4, 3);
        opening[-441496881] = pair<int, int>(4, 8);
        opening[-1166327] = pair<int, int>(2, 3);
        opening[-1544820960] = pair<int, int>(9, 3);
        opening[1797423509] = pair<int, int>(2, 4);
        opening[1788558385] = pair<int, int>(2, 5);
        opening[-1081240776] = pair<int, int>(8, 4);
        opening[552288294] = pair<int, int>(3, 2);
        opening[1152802516] = pair<int, int>(4, 2);
        opening[452782103] = pair<int, int>(5, 2);
        opening[-636408233] = pair<int, int>(7, 4);
        opening[1995179992] = pair<int, int>(5, 2);
        opening[-1241398888] = pair<int, int>(3, 4);
        opening[-1257788954] = pair<int, int>(4, 5);
        opening[-1608614680] = pair<int, int>(5, 7);
        opening[608525250] = pair<int, int>(4, 5);
        opening[616828914] = pair<int, int>(3, 4);
        opening[-523156470] = pair<int, int>(5, 4);
        opening[-1398089236] = pair<int, int>(5, 4);
        opening[993765538] = pair<int, int>(3, 5);
        opening[350245913] = pair<int, int>(4, 7);
        opening[-587495761] = pair<int, int>(7, 4);
        opening[-1368914824] = pair<int, int>(5, 3);
        opening[-1232660706] = pair<int, int>(3, 8);
        opening[-376155880] = pair<int, int>(3, 7);
        opening[1516450428] = pair<int, int>(2, 7);
        opening[560065433] = pair<int, int>(2, 7);
        opening[1627386153] = pair<int, int>(2, 7);
        opening[-873069897] = pair<int, int>(4, 7);
        opening[5770314] = pair<int, int>(5, 7);
        opening[1390890518] = pair<int, int>(3, 7);
        opening[-1014849829] = pair<int, int>(3, 4);
        opening[158576692] = pair<int, int>(3, 7);
        opening[1139662036] = pair<int, int>(6, 8);
        opening[-866174822] = pair<int, int>(2, 4);
        opening[1475808419] = pair<int, int>(3, 7);
        opening[493445187] = pair<int, int>(3, 4);
        opening[-1971803142] = pair<int, int>(3, 6);
        opening[-1279997161] = pair<int, int>(3, 8);
        opening[-324563695] = pair<int, int>(3, 7);
        opening[-386439414] = pair<int, int>(5, 4);
        opening[759162634] = pair<int, int>(3, 4);
        opening[-1152683566] = pair<int, int>(2, 4);
        opening[-1158493066] = pair<int, int>(3, 6);
        opening[-586439095] = pair<int, int>(1, 4);
        opening[1750587509] = pair<int, int>(2, 5);
        opening[-60501317] = pair<int, int>(1, 6);
        opening[960280189] = pair<int, int>(2, 6);
        opening[-1526433608] = pair<int, int>(3, 5);
        opening[326952567] = pair<int, int>(4, 6);
        opening[1289746430] = pair<int, int>(2, 6);
        opening[-1171379021] = pair<int, int>(4, 5);
        opening[833502795] = pair<int, int>(3, 5);
        opening[-1050438562] = pair<int, int>(2, 4);
        opening[1332059734] = pair<int, int>(2, 7);
        opening[1257247479] = pair<int, int>(4, 3);
        opening[500336932] = pair<int, int>(2, 3);
        opening[-728247087] = pair<int, int>(2, 5);
        opening[679462606] = pair<int, int>(1, 4);
        opening[1706483633] = pair<int, int>(4, 1);
        opening[109809425] = pair<int, int>(4, 6);
        opening[2070788321] = pair<int, int>(3, 5);
        opening[541041966] = pair<int, int>(2, 3);
        opening[849164735] = pair<int, int>(1, 4);
        opening[-293210048] = pair<int, int>(2, 6);
        opening[738194483] = pair<int, int>(4, 5);
        opening[-1755687023] = pair<int, int>(4, 6);
        opening[-1592231303] = pair<int, int>(4, 7);
        opening[-478363153] = pair<int, int>(3, 6);
        opening[-1875651325] = pair<int, int>(3, 8);
        opening[-177611820] = pair<int, int>(3, 5);
        opening[-475263838] = pair<int, int>(5, 3);
        opening[1865625350] = pair<int, int>(4, 6);
        opening[814759567] = pair<int, int>(3, 6);
        opening[195649103] = pair<int, int>(3, 7);
        opening[1833294679] = pair<int, int>(3, 4);
        opening[-250602746] = pair<int, int>(4, 5);
        opening[1138698851] = pair<int, int>(3, 3);
        opening[-786511278] = pair<int, int>(6, 7);
        opening[168603419] = pair<int, int>(4, 3);
        opening[94663105] = pair<int, int>(4, 7);
        opening[-376010589] = pair<int, int>(4, 6);
        opening[-1238001366] = pair<int, int>(3, 4);
        opening[-208813990] = pair<int, int>(6, 7);
        opening[1779370186] = pair<int, int>(3, 4);
        opening[-55383378] = pair<int, int>(3, 2);
        opening[-1247243101] = pair<int, int>(3, 9);
        opening[-1416555709] = pair<int, int>(4, 2);
        opening[-170154624] = pair<int, int>(5, 2);
        opening[1729055470] = pair<int, int>(2, 3);
        opening[1974326911] = pair<int, int>(2, 4);
        opening[1947495387] = pair<int, int>(2, 5);
        opening[1899343339] = pair<int, int>(4, 7);
        opening[-72622985] = pair<int, int>(2, 5);
        opening[-24503737] = pair<int, int>(4, 3);
        opening[408690883] = pair<int, int>(5, 4);
        opening[-799431796] = pair<int, int>(7, 5);
        opening[-1067281106] = pair<int, int>(5, 4);
        opening[-540215026] = pair<int, int>(4, 3);
        opening[-543406830] = pair<int, int>(4, 5);
        opening[-945627844] = pair<int, int>(4, 5);
        opening[979301827] = pair<int, int>(5, 3);
        opening[207631620] = pair<int, int>(7, 4);
        opening[-690560030] = pair<int, int>(4, 7);
        opening[1061428833] = pair<int, int>(3, 5);
        opening[696371479] = pair<int, int>(8, 3);
        opening[1262680132] = pair<int, int>(7, 3);
        opening[-66977288] = pair<int, int>(7, 2);
        opening[95602048] = pair<int, int>(7, 2);
        opening[-847207372] = pair<int, int>(7, 2);
        opening[-1033855695] = pair<int, int>(7, 4);
        opening[-1268057767] = pair<int, int>(7, 5);
        opening[-1701568623] = pair<int, int>(7, 3);
        opening[-393925413] = pair<int, int>(4, 3);
        opening[426916508] = pair<int, int>(7, 3);
        opening[210216123] = pair<int, int>(8, 6);
        opening[1925759263] = pair<int, int>(4, 2);
        opening[-489683798] = pair<int, int>(7, 3);
        opening[-148530547] = pair<int, int>(4, 3);
        opening[997447074] = pair<int, int>(6, 3);
        opening[-1723653350] = pair<int, int>(8, 3);
        opening[-75492791] = pair<int, int>(7, 3);
        opening[638882820] = pair<int, int>(4, 5);
        opening[271626892] = pair<int, int>(7, 8);
        opening[-559670064] = pair<int, int>(7, 9);
        opening[468782459] = pair<int, int>(5, 8);
        opening[-672492952] = pair<int, int>(7, 10);
        opening[1370455390] = pair<int, int>(6, 9);
        opening[-1475148975] = pair<int, int>(5, 10);
        opening[-1021886295] = pair<int, int>(5, 9);
        opening[197444790] = pair<int, int>(6, 8);
        opening[-1437944254] = pair<int, int>(5, 7);
        opening[-1553921476] = pair<int, int>(5, 9);
        opening[1086534339] = pair<int, int>(6, 7);
        opening[1983162764] = pair<int, int>(6, 8);
        opening[-1238585187] = pair<int, int>(7, 9);
        opening[-1548669435] = pair<int, int>(4, 9);
        opening[-776013226] = pair<int, int>(8, 7);
        opening[-228312763] = pair<int, int>(8, 9);
        opening[-247401174] = pair<int, int>(6, 9);
        opening[-1935329948] = pair<int, int>(7, 10);
        opening[-192935262] = pair<int, int>(10, 7);
        opening[329160808] = pair<int, int>(5, 7);
        opening[-223316229] = pair<int, int>(6, 8);
        opening[-571383400] = pair<int, int>(8, 9);
        opening[1401018198] = pair<int, int>(7, 10);
        opening[1474353089] = pair<int, int>(5, 9);
        opening[1015483794] = pair<int, int>(6, 7);
        opening[1876600633] = pair<int, int>(5, 7);
        opening[1292311836] = pair<int, int>(4, 7);
        opening[-259173188] = pair<int, int>(5, 8);
        opening[1723823022] = pair<int, int>(3, 8);
        opening[534784505] = pair<int, int>(6, 8);
        opening[-516038526] = pair<int, int>(8, 6);
        opening[-1317658468] = pair<int, int>(5, 7);
        opening[-1201824542] = pair<int, int>(5, 8);
        opening[-1015494917] = pair<int, int>(4, 8);
        opening[-408558540] = pair<int, int>(7, 8);
        opening[-1821670276] = pair<int, int>(6, 7);
        opening[779074544] = pair<int, int>(8, 8);
        opening[-483541687] = pair<int, int>(4, 5);
        opening[420177140] = pair<int, int>(8, 7);
        opening[-537382680] = pair<int, int>(4, 7);
        opening[-1801081396] = pair<int, int>(5, 7);
        opening[-1651633742] = pair<int, int>(7, 8);
        opening[760831824] = pair<int, int>(4, 5);
        opening[-1656739368] = pair<int, int>(7, 8);
        opening[-2042679951] = pair<int, int>(9, 8);
        opening[-1210346596] = pair<int, int>(2, 8);
        opening[-1195022479] = pair<int, int>(9, 7);
        opening[-1558055777] = pair<int, int>(9, 6);
        opening[1350695628] = pair<int, int>(8, 9);
        opening[-554611710] = pair<int, int>(7, 9);
        opening[463756713] = pair<int, int>(6, 9);
        opening[195049320] = pair<int, int>(4, 7);
        opening[-882060646] = pair<int, int>(6, 9);
        opening[-613894053] = pair<int, int>(8, 7);
        opening[-1357336314] = pair<int, int>(7, 6);
        opening[-763783924] = pair<int, int>(6, 4);
        opening[-1676191824] = pair<int, int>(7, 6);
        opening[-1462585425] = pair<int, int>(8, 7);
        opening[-614889035] = pair<int, int>(6, 7);
        opening[1308725131] = pair<int, int>(6, 7);
        opening[-902689257] = pair<int, int>(8, 6);
        opening[266340226] = pair<int, int>(7, 4);
        opening[631982074] = pair<int, int>(4, 7);
        opening[-458336030] = pair<int, int>(6, 8);
        opening[443781529] = pair<int, int>(8, 3);
        opening[2024841418] = pair<int, int>(8, 4);
        opening[-1652003191] = pair<int, int>(9, 4);
        opening[1200440094] = pair<int, int>(9, 4);
        opening[1163884685] = pair<int, int>(9, 4);
        opening[2139291643] = pair<int, int>(7, 4);
        opening[159947667] = pair<int, int>(6, 4);
        opening[-1578228660] = pair<int, int>(8, 4);
        opening[770077645] = pair<int, int>(8, 7);
        opening[759368675] = pair<int, int>(8, 4);
        opening[-697068135] = pair<int, int>(5, 3);
        opening[-1853925429] = pair<int, int>(9, 7);
        opening[-1053897256] = pair<int, int>(8, 4);
        opening[975066018] = pair<int, int>(8, 7);
        opening[445771257] = pair<int, int>(8, 5);
        opening[614547920] = pair<int, int>(8, 3);
        opening[1180856451] = pair<int, int>(8, 4);
        opening[1217639068] = pair<int, int>(6, 7);
        opening[-913030234] = pair<int, int>(8, 3);
        opening[-108357124] = pair<int, int>(3, 8);
        opening[-834831989] = pair<int, int>(3, 7);
        opening[325453762] = pair<int, int>(4, 4);
        opening[465914631] = pair<int, int>(3, 6);
        opening[-1931614176] = pair<int, int>(4, 6);
        opening[1733933483] = pair<int, int>(5, 4);
        opening[49146060] = pair<int, int>(5, 3);
        opening[1076438932] = pair<int, int>(7, 3);
        opening[1388885924] = pair<int, int>(0, 8);
        opening[-1659788707] = pair<int, int>(6, 4);
        opening[1574199498] = pair<int, int>(8, 2);
        opening[-810327457] = pair<int, int>(7, 5);
        opening[1377126305] = pair<int, int>(3, 6);
        opening[-1696531168] = pair<int, int>(2, 7);
        opening[-624427126] = pair<int, int>(8, 4);
        opening[1038664] = pair<int, int>(6, 3);
        opening[-471951914] = pair<int, int>(5, 4);
        opening[-2114881532] = pair<int, int>(7, 7);
        opening[-1695893114] = pair<int, int>(7, 8);
        opening[-1331830748] = pair<int, int>(6, 3);
        opening[-460376685] = pair<int, int>(7, 5);
        opening[1862568396] = pair<int, int>(6, 7);
        opening[-1404499217] = pair<int, int>(8, 2);
        opening[-398241686] = pair<int, int>(6, 4);
        opening[-1567223967] = pair<int, int>(8, 7);
        opening[1988271017] = pair<int, int>(7, 3);
        opening[-1409081921] = pair<int, int>(7, 5);
        opening[-61700180] = pair<int, int>(3, 7);
        opening[-1111416698] = pair<int, int>(1, 5);
        opening[679908093] = pair<int, int>(7, 3);
        opening[-1744379035] = pair<int, int>(7, 5);
        opening[213859672] = pair<int, int>(3, 5);
        opening[-364417997] = pair<int, int>(3, 2);
        opening[-19008915] = pair<int, int>(5, 4);
        opening[-1299799403] = pair<int, int>(6, 7);
        opening[-656065164] = pair<int, int>(8, 7);
        opening[94243221] = pair<int, int>(4, 3);
        opening[-1845959984] = pair<int, int>(6, 7);
        opening[1193153421] = pair<int, int>(7, 5);
        opening[740598827] = pair<int, int>(9, 8);
        opening[-774885743] = pair<int, int>(4, 3);
        opening[1684584568] = pair<int, int>(6, 4);
        opening[-657151883] = pair<int, int>(9, 8);
        opening[968435736] = pair<int, int>(4, 6);
        opening[-736430539] = pair<int, int>(8, 4);
        opening[1376792] = pair<int, int>(4, 4);
        opening[2069551642] = pair<int, int>(4, 4);
        opening[-1130296791] = pair<int, int>(7, 3);
        opening[-3899316] = pair<int, int>(7, 4);
        opening[-767681990] = pair<int, int>(8, 4);
        opening[-838557254] = pair<int, int>(3, 6);
        opening[-882727113] = pair<int, int>(4, 2);
        opening[103935511] = pair<int, int>(6, 4);
        opening[-2025638768] = pair<int, int>(3, 8);
        opening[1346756382] = pair<int, int>(9, 8);
        opening[1226149537] = pair<int, int>(6, 4);
        opening[214429487] = pair<int, int>(3, 7);
        opening[-778268314] = pair<int, int>(3, 6);
        opening[-1076267155] = pair<int, int>(4, 8);
        opening[27533937] = pair<int, int>(6, 8);
        opening[139612889] = pair<int, int>(5, 2);
        opening[-538892184] = pair<int, int>(4, 2);
        opening[143606798] = pair<int, int>(4, 4);
        opening[1932837874] = pair<int, int>(4, 4);
        opening[1283708465] = pair<int, int>(7, 6);
        opening[-468491411] = pair<int, int>(6, 3);
        opening[802109042] = pair<int, int>(6, 4);
        opening[-192499897] = pair<int, int>(7, 3);
        opening[-726214836] = pair<int, int>(4, 4);
        opening[1416943042] = pair<int, int>(3, 5);
        opening[1861725905] = pair<int, int>(7, 6);
        opening[416740917] = pair<int, int>(4, 4);
        opening[-1743457093] = pair<int, int>(7, 6);
        opening[474746124] = pair<int, int>(8, 5);
        opening[-2104702161] = pair<int, int>(7, 4);
        opening[-1347212251] = pair<int, int>(4, 5);
        opening[1223794938] = pair<int, int>(2, 6);
        opening[-1040864723] = pair<int, int>(3, 6);
        opening[462581062] = pair<int, int>(4, 3);
        opening[410802669] = pair<int, int>(3, 4);
        opening[-257157291] = pair<int, int>(5, 2);
        opening[-429463556] = pair<int, int>(5, 4);
        opening[86756312] = pair<int, int>(10, 9);
        opening[-881804554] = pair<int, int>(8, 4);
        opening[-342789306] = pair<int, int>(7, 3);
        opening[1698781153] = pair<int, int>(7, 4);
        opening[-60821112] = pair<int, int>(4, 3);
        opening[-2138346496] = pair<int, int>(3, 2);
        opening[-166927239] = pair<int, int>(4, 4);
        opening[1871228863] = pair<int, int>(3, 4);
        opening[-295518133] = pair<int, int>(3, 4);
        opening[-403536200] = pair<int, int>(7, 6);
        opening[823424997] = pair<int, int>(7, 5);
        opening[1442732854] = pair<int, int>(9, 8);
        opening[-1430489305] = pair<int, int>(7, 8);
        opening[-2134167931] = pair<int, int>(9, 8);
        opening[1638313704] = pair<int, int>(9, 5);
        opening[-2022336820] = pair<int, int>(5, 9);
        opening[1867671400] = pair<int, int>(9, 8);
        opening[-1142600911] = pair<int, int>(7, 9);
        opening[2034885672] = pair<int, int>(9, 8);
        opening[1740195689] = pair<int, int>(5, 10);
        opening[-2067960174] = pair<int, int>(8, 7);
        opening[-1648270430] = pair<int, int>(8, 3);
        opening[-1228282686] = pair<int, int>(7, 5);
        opening[582057845] = pair<int, int>(9, 5);
        opening[-1966641479] = pair<int, int>(4, 6);
        opening[537385438] = pair<int, int>(4, 3);
        opening[-1782624457] = pair<int, int>(6, 4);
        opening[632615411] = pair<int, int>(4, 7);
        opening[-609714942] = pair<int, int>(6, 4);
        opening[-1377846792] = pair<int, int>(3, 8);
        opening[-1596747965] = pair<int, int>(7, 6);
        opening[138623519] = pair<int, int>(8, 6);
        opening[1513013315] = pair<int, int>(9, 8);
        opening[1132368380] = pair<int, int>(6, 4);
        opening[-1515345448] = pair<int, int>(7, 4);
        opening[-1052277493] = pair<int, int>(9, 8);
        opening[539624806] = pair<int, int>(8, 5);
        opening[694560996] = pair<int, int>(7, 6);
        opening[-375689702] = pair<int, int>(6, 9);
        opening[1275297592] = pair<int, int>(4, 8);
        opening[359766420] = pair<int, int>(9, 5);
        opening[-606741104] = pair<int, int>(3, 8);
        opening[-185553541] = pair<int, int>(4, 8);
        opening[1450143071] = pair<int, int>(8, 3);
        opening[-88442600] = pair<int, int>(7, 5);
        opening[-1853829442] = pair<int, int>(9, 8);
        opening[943658226] = pair<int, int>(8, 6);
        opening[466470017] = pair<int, int>(3, 8);
        opening[-1330446955] = pair<int, int>(4, 8);
        opening[2094571537] = pair<int, int>(4, 7);
        opening[1574533811] = pair<int, int>(4, 7);
        opening[-1547307454] = pair<int, int>(6, 4);
        opening[-746568371] = pair<int, int>(8, 3);
        opening[1391829224] = pair<int, int>(4, 8);
        opening[-326568460] = pair<int, int>(7, 7);
        opening[1173108180] = pair<int, int>(5, 8);
        opening[1915496635] = pair<int, int>(5, 7);
        opening[1515645997] = pair<int, int>(7, 6);
        opening[-2053407129] = pair<int, int>(8, 6);
        opening[680926258] = pair<int, int>(8, 4);
        opening[1519645193] = pair<int, int>(3, 11);
        opening[1259304538] = pair<int, int>(7, 5);
        opening[-1065039355] = pair<int, int>(9, 3);
        opening[-1546493499] = pair<int, int>(6, 4);
        opening[-931981038] = pair<int, int>(5, 8);
        opening[311049967] = pair<int, int>(4, 9);
        opening[1568101198] = pair<int, int>(7, 3);
        opening[956783064] = pair<int, int>(8, 5);
        opening[-2027038516] = pair<int, int>(7, 6);
        opening[1983147858] = pair<int, int>(4, 4);
        opening[61376584] = pair<int, int>(3, 4);
        opening[-2054067525] = pair<int, int>(8, 5);
        opening[1880130397] = pair<int, int>(6, 4);
        opening[-1328201270] = pair<int, int>(4, 5);
        opening[-1137773561] = pair<int, int>(9, 3);
        opening[-1934615184] = pair<int, int>(7, 5);
        opening[444473452] = pair<int, int>(4, 3);
        opening[-1456755043] = pair<int, int>(8, 4);
        opening[-1345931643] = pair<int, int>(6, 4);
        opening[1071543088] = pair<int, int>(4, 8);
        opening[493616975] = pair<int, int>(6, 10);
        opening[121348399] = pair<int, int>(8, 4);
        opening[403293005] = pair<int, int>(6, 4);
        opening[1749839938] = pair<int, int>(6, 8);
        opening[-410342698] = pair<int, int>(9, 8);
        opening[-853550692] = pair<int, int>(7, 6);
        opening[-1363325977] = pair<int, int>(4, 5);
        opening[-747046712] = pair<int, int>(4, 3);
        opening[1476383925] = pair<int, int>(8, 7);
        opening[-1393051818] = pair<int, int>(4, 5);
        opening[602758807] = pair<int, int>(6, 4);
        opening[568943310] = pair<int, int>(3, 2);
        opening[1765278108] = pair<int, int>(8, 7);
        opening[1739395906] = pair<int, int>(7, 5);
        opening[-1305507664] = pair<int, int>(3, 2);
        opening[1761394531] = pair<int, int>(5, 7);
        opening[-1158651000] = pair<int, int>(7, 3);
        opening[-1500919388] = pair<int, int>(7, 7);
        opening[-684891430] = pair<int, int>(7, 7);
        opening[1765173052] = pair<int, int>(8, 4);
        opening[1846830666] = pair<int, int>(7, 4);
        opening[-544330209] = pair<int, int>(7, 3);
        opening[-1072807348] = pair<int, int>(5, 8);
        opening[161326857] = pair<int, int>(9, 7);
        opening[-1007188941] = pair<int, int>(7, 5);
        opening[-1208199885] = pair<int, int>(3, 8);
        opening[-566062359] = pair<int, int>(3, 2);
        opening[-1267910259] = pair<int, int>(7, 5);
        opening[-919622849] = pair<int, int>(4, 8);
        opening[2001927715] = pair<int, int>(5, 8);
        opening[-1590543218] = pair<int, int>(3, 7);
        opening[2087273159] = pair<int, int>(3, 5);
        opening[585489000] = pair<int, int>(9, 6);
        opening[1879264012] = pair<int, int>(9, 7);
        opening[-563141736] = pair<int, int>(7, 7);
        opening[-1512790940] = pair<int, int>(7, 7);
        opening[678468395] = pair<int, int>(5, 4);
        opening[-1567238990] = pair<int, int>(8, 5);
        opening[384942242] = pair<int, int>(7, 5);
        opening[-1428989095] = pair<int, int>(8, 4);
        opening[-2025937838] = pair<int, int>(7, 7);
        opening[-478268879] = pair<int, int>(6, 8);
        opening[1366326164] = pair<int, int>(5, 4);
        opening[323695497] = pair<int, int>(7, 7);
        opening[1997252074] = pair<int, int>(5, 4);
        opening[-212182629] = pair<int, int>(6, 3);
        opening[-589742287] = pair<int, int>(7, 4);
        opening[-857109059] = pair<int, int>(6, 7);
        opening[340109443] = pair<int, int>(5, 9);
        opening[-254880107] = pair<int, int>(5, 8);
        opening[1314544868] = pair<int, int>(8, 7);
        opening[-118721689] = pair<int, int>(7, 8);
        opening[822640939] = pair<int, int>(9, 6);
        opening[109789110] = pair<int, int>(7, 6);
        opening[-1347055850] = pair<int, int>(2, 1);
        opening[1496784418] = pair<int, int>(7, 3);
        opening[710845580] = pair<int, int>(8, 4);
        opening[1859949149] = pair<int, int>(7, 4);
        opening[-1234956550] = pair<int, int>(9, 8);
        opening[-1289446096] = pair<int, int>(7, 7);
        opening[611166337] = pair<int, int>(7, 8);
        opening[1442293012] = pair<int, int>(7, 8);
        opening[-620899522] = pair<int, int>(5, 4);
        opening[1441217279] = pair<int, int>(6, 4);
        opening[-1883948424] = pair<int, int>(3, 2);
        opening[643245104] = pair<int, int>(3, 4);
        opening[-1603352893] = pair<int, int>(3, 2);
        opening[2059268368] = pair<int, int>(6, 2);
        opening[1658049826] = pair<int, int>(2, 6);
        opening[1290195170] = pair<int, int>(3, 2);
        opening[-711466097] = pair<int, int>(2, 4);
        opening[-1988810431] = pair<int, int>(3, 2);
        opening[-1173369213] = pair<int, int>(1, 6);
        opening[-1046197741] = pair<int, int>(4, 3);
        opening[-223352060] = pair<int, int>(8, 3);
        opening[-1575162984] = pair<int, int>(6, 4);
        opening[167430537] = pair<int, int>(6, 2);
        opening[899249818] = pair<int, int>(5, 7);
        opening[-389434221] = pair<int, int>(8, 7);
        opening[-430135731] = pair<int, int>(7, 5);
        opening[1255831893] = pair<int, int>(4, 7);
        opening[-2017706179] = pair<int, int>(7, 5);
        opening[-1028574882] = pair<int, int>(3, 8);
        opening[-1847004165] = pair<int, int>(5, 4);
        opening[453885026] = pair<int, int>(5, 3);
        opening[1474496166] = pair<int, int>(3, 2);
        opening[1036824002] = pair<int, int>(7, 5);
        opening[631409136] = pair<int, int>(7, 4);
        opening[-1013385] = pair<int, int>(3, 2);
        opening[623099556] = pair<int, int>(6, 3);
        opening[1284064742] = pair<int, int>(5, 4);
        opening[-591378812] = pair<int, int>(2, 5);
        opening[696659658] = pair<int, int>(3, 7);
        opening[-190726740] = pair<int, int>(6, 2);
        opening[-1262791370] = pair<int, int>(3, 8);
        opening[-166637284] = pair<int, int>(3, 7);
        opening[956425721] = pair<int, int>(8, 3);
        opening[-1639369726] = pair<int, int>(6, 4);
        opening[-1672673189] = pair<int, int>(3, 2);
        opening[-1085701543] = pair<int, int>(5, 3);
        opening[1148649251] = pair<int, int>(3, 8);
        opening[1942036308] = pair<int, int>(3, 7);
        opening[720148974] = pair<int, int>(4, 7);
        opening[39993617] = pair<int, int>(4, 7);
        opening[-821819527] = pair<int, int>(7, 5);
        opening[1539168580] = pair<int, int>(8, 3);
        opening[-1887371884] = pair<int, int>(3, 8);
        opening[37375323] = pair<int, int>(8, 3);
        opening[1635603876] = pair<int, int>(8, 4);
        opening[1284549295] = pair<int, int>(7, 7);
        opening[350335678] = pair<int, int>(8, 5);
        opening[1263295603] = pair<int, int>(7, 5);
        opening[-486489044] = pair<int, int>(6, 8);
        opening[-1044820897] = pair<int, int>(4, 8);
        opening[-466306363] = pair<int, int>(11, 3);
        opening[1792633551] = pair<int, int>(5, 7);
        opening[1487370772] = pair<int, int>(3, 9);
        opening[1107585208] = pair<int, int>(4, 6);
        opening[-372064652] = pair<int, int>(8, 5);
        opening[-688956000] = pair<int, int>(9, 4);
        opening[-86528898] = pair<int, int>(3, 7);
        opening[-2146496255] = pair<int, int>(5, 8);
        opening[888142970] = pair<int, int>(6, 7);
        opening[1746505915] = pair<int, int>(4, 4);
        opening[2130707847] = pair<int, int>(4, 3);
        opening[-777973930] = pair<int, int>(5, 8);
        opening[1572178424] = pair<int, int>(4, 6);
        opening[-69330519] = pair<int, int>(5, 4);
        opening[-1641217369] = pair<int, int>(3, 9);
        opening[120444811] = pair<int, int>(5, 7);
        opening[-190409593] = pair<int, int>(3, 4);
        opening[803563352] = pair<int, int>(4, 8);
        opening[1073517409] = pair<int, int>(4, 6);
        opening[-1537956008] = pair<int, int>(8, 4);
        opening[-672355251] = pair<int, int>(10, 6);
        opening[894682960] = pair<int, int>(4, 8);
        opening[-684925479] = pair<int, int>(4, 6);
        opening[-1889794872] = pair<int, int>(8, 6);
        opening[-890501985] = pair<int, int>(8, 9);
        opening[1398556529] = pair<int, int>(5, 4);
        opening[1858430873] = pair<int, int>(7, 8);
        opening[290708931] = pair<int, int>(5, 4);
        opening[-1299618874] = pair<int, int>(4, 6);
        opening[-1081531934] = pair<int, int>(2, 3);
        opening[-1199538257] = pair<int, int>(7, 8);
        opening[589121691] = pair<int, int>(5, 7);
        opening[-463634901] = pair<int, int>(2, 3);
        opening[-1611436150] = pair<int, int>(7, 5);
        opening[1917201908] = pair<int, int>(3, 7);
        opening[-1343140912] = pair<int, int>(7, 7);
        opening[-586808869] = pair<int, int>(7, 7);
        opening[1662846013] = pair<int, int>(4, 8);
        opening[1181539131] = pair<int, int>(4, 7);
        opening[-433211310] = pair<int, int>(3, 7);
        opening[-168463141] = pair<int, int>(8, 5);
        opening[619654492] = pair<int, int>(7, 9);
        opening[1000005238] = pair<int, int>(5, 7);
        opening[-934576820] = pair<int, int>(8, 3);
        opening[6797254] = pair<int, int>(2, 3);
        opening[-579208467] = pair<int, int>(5, 7);
        opening[-2135494840] = pair<int, int>(8, 4);
        opening[-1386540989] = pair<int, int>(8, 5);
        opening[1784357445] = pair<int, int>(7, 3);
        opening[1248549454] = pair<int, int>(5, 3);
        opening[509439997] = pair<int, int>(6, 9);
        opening[1149719246] = pair<int, int>(7, 9);
        opening[-1854812213] = pair<int, int>(7, 7);
        opening[2036978118] = pair<int, int>(7, 7);
        opening[-2125772745] = pair<int, int>(4, 5);
        opening[275191348] = pair<int, int>(5, 8);
        opening[1302292183] = pair<int, int>(5, 7);
        opening[620741760] = pair<int, int>(4, 8);
        opening[-1702719076] = pair<int, int>(7, 7);
        opening[-20838401] = pair<int, int>(8, 6);
        opening[797123320] = pair<int, int>(4, 5);
        opening[-1156203020] = pair<int, int>(7, 7);
        opening[-548066409] = pair<int, int>(4, 5);
        opening[647217324] = pair<int, int>(3, 6);
        opening[-279852419] = pair<int, int>(4, 7);
        opening[-1709919928] = pair<int, int>(7, 6);
        opening[-875738317] = pair<int, int>(9, 5);
        opening[2118154817] = pair<int, int>(8, 5);
        opening[224232697] = pair<int, int>(7, 8);
        opening[1408254724] = pair<int, int>(8, 7);
        opening[1744158174] = pair<int, int>(6, 9);
        opening[173333172] = pair<int, int>(6, 7);
        opening[1251449605] = pair<int, int>(1, 2);
        opening[-1757108411] = pair<int, int>(3, 7);
        opening[1826246871] = pair<int, int>(4, 8);
        opening[1132042488] = pair<int, int>(4, 7);
        opening[-1586112900] = pair<int, int>(7, 8);
        opening[2012154076] = pair<int, int>(8, 9);
        opening[826820255] = pair<int, int>(7, 7);
        opening[-1275357242] = pair<int, int>(8, 7);
        opening[1872582621] = pair<int, int>(8, 7);
        opening[2032627003] = pair<int, int>(4, 6);
        opening[-1675461134] = pair<int, int>(2, 3);
        opening[79542244] = pair<int, int>(4, 3);
        opening[-1440957131] = pair<int, int>(2, 3);
        opening[-776819564] = pair<int, int>(2, 6);
        opening[1507621514] = pair<int, int>(6, 2);
        opening[1306301240] = pair<int, int>(2, 3);
        opening[-973036539] = pair<int, int>(4, 2);
        opening[-229997175] = pair<int, int>(2, 3);
        opening[-1362527131] = pair<int, int>(6, 1);
        opening[-180102204] = pair<int, int>(3, 4);
        opening[-1243740073] = pair<int, int>(3, 8);
        opening[1971507653] = pair<int, int>(4, 6);
        opening[-534947337] = pair<int, int>(2, 6);
        opening[-1048961291] = pair<int, int>(7, 5);
        opening[2139868588] = pair<int, int>(7, 8);
        opening[-468256104] = pair<int, int>(5, 7);
        opening[111733071] = pair<int, int>(7, 4);
        opening[-743920886] = pair<int, int>(5, 7);
        opening[946124952] = pair<int, int>(8, 3);
        opening[513880387] = pair<int, int>(4, 5);
        opening[-1886556352] = pair<int, int>(3, 5);
        opening[1948725023] = pair<int, int>(2, 3);
        opening[-1455857100] = pair<int, int>(5, 7);
        opening[559395882] = pair<int, int>(4, 7);
        opening[-1001216797] = pair<int, int>(2, 3);
        opening[-1073957566] = pair<int, int>(3, 6);
        opening[399425904] = pair<int, int>(4, 5);
        opening[-1672555640] = pair<int, int>(5, 2);
        opening[-748526801] = pair<int, int>(7, 3);
        opening[-1409741074] = pair<int, int>(2, 6);
        opening[-204818843] = pair<int, int>(8, 3);
        opening[-284470499] = pair<int, int>(7, 3);
        opening[-1009488833] = pair<int, int>(3, 8);
        opening[254609747] = pair<int, int>(4, 6);
        opening[36522871] = pair<int, int>(2, 3);
        opening[-713829743] = pair<int, int>(3, 5);
        opening[200597143] = pair<int, int>(8, 3);
        opening[921055862] = pair<int, int>(7, 3);
        opening[2112639086] = pair<int, int>(7, 4);
        opening[1306550437] = pair<int, int>(7, 4);
        opening[-1730141472] = pair<int, int>(5, 7);
        opening[1222715570] = pair<int, int>(3, 8);
        opening[1059627450] = pair<int, int>(7, 3);
        opening[521722289] = pair<int, int>(4, 4);
        opening[607346744] = pair<int, int>(6, 3);
        opening[-1450733596] = pair<int, int>(6, 4);
        opening[1758967812] = pair<int, int>(3, 5);
        opening[-1872750730] = pair<int, int>(3, 7);
        opening[-419324778] = pair<int, int>(8, 0);
        opening[28658485] = pair<int, int>(4, 6);
        opening[-1478567068] = pair<int, int>(2, 8);
        opening[1373214409] = pair<int, int>(5, 7);
        opening[-1799735739] = pair<int, int>(6, 3);
        opening[-632681599] = pair<int, int>(7, 2);
        opening[116229102] = pair<int, int>(4, 8);
        opening[1991564085] = pair<int, int>(3, 6);
        opening[-146723745] = pair<int, int>(4, 5);
        opening[1696482525] = pair<int, int>(7, 7);
        opening[-1327073711] = pair<int, int>(8, 7);
        opening[-1668163702] = pair<int, int>(3, 6);
        opening[-243936901] = pair<int, int>(5, 7);
        opening[-1022639712] = pair<int, int>(7, 6);
        opening[-2071125909] = pair<int, int>(2, 8);
        opening[-998045778] = pair<int, int>(4, 6);
        opening[-829809721] = pair<int, int>(7, 8);
        opening[-1696033581] = pair<int, int>(3, 7);
        opening[1427582195] = pair<int, int>(5, 7);
        opening[225472003] = pair<int, int>(7, 3);
        opening[1930813974] = pair<int, int>(5, 1);
        opening[1659410431] = pair<int, int>(3, 7);
        opening[-1661739329] = pair<int, int>(5, 7);
        opening[1290982637] = pair<int, int>(5, 3);
        opening[-914143862] = pair<int, int>(2, 3);
        opening[-308700699] = pair<int, int>(7, 6);
        opening[-1549350453] = pair<int, int>(3, 4);
        opening[744261701] = pair<int, int>(7, 6);
        opening[1898427363] = pair<int, int>(5, 7);
        opening[-1125752585] = pair<int, int>(8, 9);
        opening[-2102595345] = pair<int, int>(3, 4);
        opening[1240921865] = pair<int, int>(4, 6);
        opening[1316442249] = pair<int, int>(8, 9);
        opening[33258973] = pair<int, int>(6, 4);
        opening[-967512878] = pair<int, int>(4, 8);
        opening[944350623] = pair<int, int>(4, 4);
        opening[-1417024128] = pair<int, int>(4, 4);
        opening[1819426227] = pair<int, int>(3, 7);
        opening[-1084091682] = pair<int, int>(4, 7);
        opening[-201788015] = pair<int, int>(4, 8);
        opening[-1831763067] = pair<int, int>(6, 3);
        opening[1596537493] = pair<int, int>(2, 4);
        opening[233159900] = pair<int, int>(4, 6);
        opening[-527342327] = pair<int, int>(8, 3);
        opening[574376487] = pair<int, int>(8, 9);
        opening[1630110243] = pair<int, int>(4, 6);
        opening[1802569948] = pair<int, int>(7, 3);
        opening[1262579927] = pair<int, int>(6, 3);
        opening[-1253191260] = pair<int, int>(8, 4);
        opening[-1734332753] = pair<int, int>(8, 6);
        opening[554579711] = pair<int, int>(2, 5);
        opening[2108309753] = pair<int, int>(2, 4);
        opening[1622313586] = pair<int, int>(4, 4);
        opening[-2002086785] = pair<int, int>(4, 4);
        opening[1122342930] = pair<int, int>(6, 7);
        opening[478146815] = pair<int, int>(3, 6);
        opening[750120753] = pair<int, int>(4, 6);
        opening[-1204347421] = pair<int, int>(3, 7);
        opening[1701602218] = pair<int, int>(4, 4);
        opening[-441556700] = pair<int, int>(5, 3);
        opening[-425522034] = pair<int, int>(6, 7);
        opening[-1999137275] = pair<int, int>(4, 4);
        opening[135632011] = pair<int, int>(6, 7);
        opening[-1153160350] = pair<int, int>(5, 8);
        opening[1939411742] = pair<int, int>(4, 7);
        opening[1003699629] = pair<int, int>(5, 4);
        opening[2016401894] = pair<int, int>(6, 2);
        opening[316954870] = pair<int, int>(6, 3);
        opening[-1571326357] = pair<int, int>(3, 4);
        opening[-754748651] = pair<int, int>(4, 3);
        opening[1691092701] = pair<int, int>(2, 5);
        opening[-1675762061] = pair<int, int>(4, 5);
        opening[1074564509] = pair<int, int>(9, 10);
        opening[-1543339330] = pair<int, int>(4, 8);
        opening[-1603107046] = pair<int, int>(3, 7);
        opening[1891349257] = pair<int, int>(4, 7);
        opening[1872263805] = pair<int, int>(2, 3);
        opening[-23538822] = pair<int, int>(4, 4);
        opening[1253675545] = pair<int, int>(4, 3);
        opening[1995834724] = pair<int, int>(4, 3);
        opening[-1165378274] = pair<int, int>(5, 7);
        opening[1426933875] = pair<int, int>(8, 9);
        opening[-1129783235] = pair<int, int>(8, 7);
        opening[-1864416794] = pair<int, int>(8, 9);
        opening[-548180814] = pair<int, int>(5, 9);
        opening[-2121892368] = pair<int, int>(9, 5);
        opening[946345376] = pair<int, int>(8, 9);
        opening[-1156163468] = pair<int, int>(9, 7);
        opening[-2023481518] = pair<int, int>(8, 9);
        opening[-660900828] = pair<int, int>(10, 5);
        opening[-1188130998] = pair<int, int>(7, 8);
        opening[1169355651] = pair<int, int>(3, 8);
        opening[1844851740] = pair<int, int>(5, 7);
        opening[1234325318] = pair<int, int>(5, 9);
        opening[1382971299] = pair<int, int>(6, 4);
        opening[-1635944688] = pair<int, int>(3, 4);
        opening[1428648182] = pair<int, int>(4, 6);
        opening[-154922341] = pair<int, int>(7, 4);
        opening[1643509513] = pair<int, int>(4, 6);
        opening[-938982580] = pair<int, int>(8, 3);
        opening[1912935412] = pair<int, int>(6, 7);
        opening[748649241] = pair<int, int>(6, 8);
        opening[2001235466] = pair<int, int>(8, 9);
        opening[878552590] = pair<int, int>(4, 6);
        opening[1787344716] = pair<int, int>(4, 7);
        opening[-2062794207] = pair<int, int>(8, 9);
        opening[-897552523] = pair<int, int>(5, 8);
        opening[1494882032] = pair<int, int>(6, 7);
        opening[1877839295] = pair<int, int>(9, 6);
        opening[-726748011] = pair<int, int>(8, 4);
        opening[396735559] = pair<int, int>(5, 9);
        opening[61230513] = pair<int, int>(8, 3);
        opening[-1071572821] = pair<int, int>(8, 4);
        opening[867735531] = pair<int, int>(3, 8);
        opening[-863774346] = pair<int, int>(5, 7);
        opening[21105250] = pair<int, int>(8, 9);
        opening[1587850937] = pair<int, int>(6, 8);
        opening[-1240023971] = pair<int, int>(8, 3);
        opening[-716069726] = pair<int, int>(8, 4);
        opening[-675165527] = pair<int, int>(7, 4);
        opening[-268303761] = pair<int, int>(7, 4);
        opening[1731454973] = pair<int, int>(4, 6);
        opening[1061371628] = pair<int, int>(3, 8);
    }

    loop();

    return 0;
}