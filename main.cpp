// -std=c++11
// 导入程序所需要的标准库 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <conio.h>
#include <cstring>
#include <windows.h>
#include <cstdlib>
using namespace std;
 
struct board_state // 局面的胜负 
{
    string cnt, result;
};
vector<board_state> b[9]; // 电脑库 
char board[3][3]; // 棋盘 
char board_raw[3][3] = {{'7','8','9'},{'4','5','6'},{'1','2','3'}}; // 原始棋盘 
char turn; // 该谁落子 
char mode; // 游戏模式 
bool random; // 电脑难度 
string cnt; // 棋盘串 

void vis_set(string cnt, bool vis[8][3]) // 将棋子串映射进矩阵 
{
    for(int i = 0; i < cnt.size(); i++){
        switch(cnt[i]){
            case '1': vis[0][0] = vis[3][0] = vis[6][0] = true; break;
            case '2': vis[0][1] = vis[4][0] = true; break;
            case '3': vis[0][2] = vis[5][0] = vis[7][0] = true; break;
            case '4': vis[1][0] = vis[3][1] = true; break;
            case '5': vis[1][1] = vis[4][1] = vis[6][1] = vis[7][1] = true; break;
            case '6': vis[1][2] = vis[5][1] = true; break;
            case '7': vis[2][0] = vis[3][2] = vis[7][2] = true; break;
            case '8': vis[2][1] = vis[4][2] = true; break;
            case '9': vis[2][2] = vis[5][2] = vis[6][2] = true; break;
        }
    }
}

string cnt_check(string cnt) // 检查当前局面的胜负 
{
    string circle, cross;
    circle = cross = "";
    for(int i = 0; i < cnt.size(); i++){
        if(i % 2 == 0) circle += cnt[i];
        else cross += cnt[i];
    }

    bool vis_circle[8][3], vis_cross[8][3];
    memset(vis_circle, false, sizeof(vis_circle));
    memset(vis_cross, false, sizeof(vis_cross));

    vis_set(circle, vis_circle);
    vis_set(cross, vis_cross);

    for(int j = 0; j < 8; j++) if(vis_circle[j][0] && vis_circle[j][1] && vis_circle[j][2]) return "circle";
    for(int j = 0; j < 8; j++) if(vis_cross[j][0] && vis_cross[j][1] && vis_cross[j][2]) return "cross";
    if(cnt.size() == 9) return "draw";
    return "none";
}

void draw_board() // 在屏幕上绘制棋盘 
{
    system("cls");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    WORD original_attrs = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    if(GetConsoleScreenBufferInfo(hConsole, &csbi)) original_attrs = csbi.wAttributes;

    for(int i = 0; i < 3; i++){
        cout << " ";
        for(int j = 0; j < 3; j++){
            char c = board[i][j];
            if(c == 'O'){
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
                cout << c;
                SetConsoleTextAttribute(hConsole, original_attrs);
            }
            else if(c == 'X'){
                SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                cout << c;
                SetConsoleTextAttribute(hConsole, original_attrs);
            }
            else cout << c;
            if(j < 2) cout << " | ";
        }
        cout << endl;
        if(i < 2) cout << "---+---+---" << endl;
    }
}

bool place_mark(char turn, char key) // 在棋盘上落子 
{
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(board[i][j] == key){
                cnt += key;
                board[i][j] = turn;
                return true;
            }
        }
    }
    return false;
}

string dfs(string cnt, fstream f[9]) // 生成电脑库 
{
    string res_cnt = cnt_check(cnt);
    if(res_cnt != "none"){
        if(!cnt.empty() && f[cnt.size() - 1].is_open()) f[cnt.size() - 1] << cnt << ' ' << res_cnt << endl;
        return res_cnt;
    }

    vector<string> res;
    for(int i = 1; i <= 9; i++){
        if(cnt.find(char('0' + i)) == string::npos) res.push_back(dfs(cnt + char('0' + i), f));
    }

    res_cnt = "draw";
    size_t count = 0;
    if(cnt.size() % 2 == 0){
        if(find(res.begin(), res.end(), "circle") != res.end()) res_cnt = "circle";
        else{
            for(size_t i = 0; i < res.size(); i++) if(res[i] == "cross") count++;
            if(count == res.size()) res_cnt = "cross";
        }
    }
    else{
        if(find(res.begin(), res.end(), "cross") != res.end()) res_cnt = "cross";
        else{
            for(size_t i = 0; i < res.size(); i++) if(res[i] == "circle") count++;
            if(count == res.size()) res_cnt = "circle";
        }
    }

    if(!cnt.empty() && f[cnt.size() - 1].is_open()) f[cnt.size() - 1] << cnt << ' ' << res_cnt << endl;
    return res_cnt;
}

bool files_check() // 检查电脑库文件是否存在 
{
    for(int i = 1; i <= 9; i++){
        string filename = to_string(i) + ".txt";
        ifstream f(filename);
        if(!f) return false;
    }
    return true;
}

void load_files(vector<board_state> b[9]) // 加载电脑库 
{
    for(int i = 0; i < 9; i++){
        string filename = to_string(i + 1) + ".txt";
        ifstream fin(filename);
        if(!fin) continue;
        string line;
        while(getline(fin, line)){
            istringstream iss(line);
            board_state state;
            iss >> state.cnt >> state.result;
            b[i].push_back(state);
        }
    }
}

char computer_move(char computer, const string & cnt, vector<board_state> b[9]) // 获取电脑最优走法
{
    vector<int> empty_cells;
    for(int i = 1; i <= 9; i++) if(cnt.find(char('0' + i)) == string::npos) empty_cells.push_back(i);

    vector<int> win_moves, draw_moves, lose_moves;
    for(int pos : empty_cells){
        string next_cnt = cnt + char('0' + pos);
        int idx = next_cnt.size() - 1;
        if(idx < 0 || idx >= 9) continue;
        auto it = find_if(b[idx].begin(), b[idx].end(), [&](const board_state & s){ return s.cnt == next_cnt; });
        if(it != b[idx].end()){
            if((computer == 'O' && it->result == "circle") || (computer == 'X' && it->result == "cross")) win_moves.push_back(pos);
            else if(it->result == "draw") draw_moves.push_back(pos);
            else lose_moves.push_back(pos);
        }
    }

    srand((unsigned int)time(nullptr));
    if(!win_moves.empty()) return char('0' + win_moves[rand() % win_moves.size()]);
    if(!draw_moves.empty()) return char('0' + draw_moves[rand() % draw_moves.size()]);
    if(!lose_moves.empty()) return char('0' + lose_moves[rand() % lose_moves.size()]);
    if(!empty_cells.empty()) return char('0' + empty_cells[rand() % empty_cells.size()]);
    return '1';
}

char random_move(char turn, const string & cnt)
{
    vector<int> empty_cells;
    for(int i = 1; i <= 9; i++) if(cnt.find(char('0' + i)) == string::npos) empty_cells.push_back(i);

    char opp = (turn == 'O') ? 'X' : 'O';
    vector<int> win_moves, block_moves;

    for(int pos : empty_cells){
        string tmp;

        // 先检查自己落子能否立即胜利
        tmp = cnt + char('0' + pos);
        if(cnt_check(tmp) == ((turn == 'O') ? "circle" : "cross")) win_moves.push_back(pos);

        // 再检查对手落子能否立即胜利
        tmp = cnt + char('0' + pos);
        string opp_tmp = tmp + char('0' + pos); // 对方下一步落子
        if(cnt_check(opp_tmp) == ((opp == 'O') ? "circle" : "cross")) block_moves.push_back(pos);
    }

    srand((unsigned int)time(nullptr));
    if(!win_moves.empty()) return char('0' + win_moves[rand() % win_moves.size()]);
    if(!block_moves.empty()) return char('0' + block_moves[rand() % block_moves.size()]);
    if(!empty_cells.empty()) return char('0' + empty_cells[rand() % empty_cells.size()]);

    return '1';
}

int main() // 主程序 
{
	// 加载电脑库 
	if(!files_check()){
        fstream f[9];
        for(int i = 0; i < 9; i++){
            string filename = to_string(i + 1) + ".txt";
            f[i].open(filename, ios::out);
        }
        dfs("", f);
        for(int i = 0; i < 9; i++) f[i].close();
    }
    load_files(b);
    
    while(true){
	    // 大循环 
	    // 初始化 
	    for(int i = 0; i < 3; i++){
			for(int j = 0; j < 3; j++){
				board[i][j] = board_raw[i][j];
			}
		}
		turn = 'O';
		cnt = "";
		random = false;
	
		// 选择游戏模式 
	    cout << "请选择你的游戏模式。1) 玩家VS电脑 2) 电脑VS玩家 3) 玩家VS玩家";
	    while(true){
	        char key = _getch();
	        if(key >= '1' && key <= '3'){ mode = key; break; }
	    }
	
		// 选择电脑难度
		if(mode == '1' || mode == '2'){
			while(true){
				cout << endl << "请选择电脑难度: 1) 简单 2) 无敌";
				char key = _getch();
				if(key == '1'){
					random = true;
					break;
				}
				if(key == '2') break;
			} 
		} 

		// 游戏主循环 
	    if(mode == '1'){ // 玩家VS电脑 
	        while(true){
	            draw_board();
	            string result = cnt_check(cnt);
	            if(result != "none"){
	                if(result == "circle") cout << "O 胜利！";
	                else if(result == "cross") cout << "X 胜利！";
	                else cout << "和局！";
	                break;
	            }
	
	            if(turn == 'O'){
	                cout << "轮到 " << turn << "，请输入1-9: ";
	                char key = _getch();
	                if(key >= '1' && key <= '9') if(place_mark(turn, key)) turn = 'X';
	            }
	            else{
	            	if(!random){
	            		char key = computer_move('X', cnt, b);
		                place_mark(turn, key);
		                turn = 'O';
					}
	                else{
	                	char key = random_move('X', cnt);
	                	place_mark(turn, key);
	                	turn = 'O';
					}
	            }
	        }
	    }
	    else if(mode == '2'){ // 电脑VS玩家 
	        char first_key = computer_move('O', cnt, b);
	        place_mark('O', first_key);
	        turn = 'X';
	
	        while(true){
	            draw_board();
	            string result = cnt_check(cnt);
	            if(result != "none"){
	                if(result == "circle") cout << "O 胜利！";
	                else if(result == "cross") cout << "X 胜利！";
	                else cout << "和局！";
	                break;
	            }
	
	            if(turn == 'O'){
	            	if(!random){
		            	char key = computer_move('O', cnt, b);
		                place_mark('O', key);
		                turn = 'X';
					}
					else{
						char key = random_move('O', cnt);
						place_mark('O', key);
						turn = 'X';
					}
	            }
	            else{
	                cout << "轮到 " << turn << "，请输入1-9: ";
	                char key = _getch();
	                if(key >= '1' && key <= '9') if(place_mark(turn, key)) turn = 'O';
	            }
	        }
	    }
	    else{ // 玩家VS玩家 
	        while(true){
	            draw_board();
	            string result = cnt_check(cnt);
	            if(result != "none"){
	                if(result == "circle") cout << "O 胜利！";
	                else if(result == "cross") cout << "X 胜利！";
	                else cout << "和局！";
	                break;
	            }
	
	            cout << "轮到 " << turn << "，请输入1-9: ";
	            char key = _getch();
	            if(key >= '1' && key <= '9') if(place_mark(turn, key)) turn = (turn == 'O') ? 'X' : 'O';
	        }
	    }
	    cout << "想要再来一局吗? Y) 是 N) 否";
	    while(true){
	    	char key = _getch();
	    	if(key == 'y') break;
	    	if(key == 'n') return 0;
		}
		system("cls");
	}
    
    return 0;
}
