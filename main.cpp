#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include "jsoncpp/json.h"
#include <vector>
using namespace std;

int board[9][9];


bool dfs_air_visit[9][9];
const int cx[]={-1,0,1,0};
const int cy[]={0,-1,0,1};

bool inBorder(int x, int y){return x>=0 && y>=0 && x<9 && y<9;}

//true: has air
bool dfs_air(int fx, int fy)
{
    dfs_air_visit[fx][fy] = true;
    bool flag=false;
    for (int dir = 0; dir < 4; dir++)
    {
        int dx=fx+cx[dir], dy=fy+cy[dir];
        if (inBorder(dx, dy))
        {
            if (board[dx][dy] == 0)
                flag=true;
            if (board[dx][dy] == board[fx][fy] && !dfs_air_visit[dx][dy])
                if (dfs_air(dx, dy))
                    flag=true;
        }
    }
    return flag;
}

//true: available
bool judgeAvailable(int fx, int fy, int col)
{
    if (board[fx][fy]) return false;
    board[fx][fy] = col;
    memset(dfs_air_visit, 0, sizeof(dfs_air_visit));
    if (!dfs_air(fx, fy))
    {
        board[fx][fy]  = 0;
        return false;
    }
    for (int dir = 0; dir < 4; dir++)
    {
        int dx=fx+cx[dir], dy=fy+cy[dir];
        if (inBorder(dx, dy))
        {
            if (board[dx][dy] && !dfs_air_visit[dx][dy])
                if (!dfs_air(dx, dy))
                {
                    board[fx][fy]  = 0;
                    return false;
                }
        }
    }
    board[fx][fy]  = 0;
    return true;
}

//计算当前选点的权利和
int valuePoint(int x,int y) {
    int value=0;
    if (judgeAvailable(x,y,-1)) {
        board[x][y]=-1;
        //若此步为白棋计算白棋的权利和
        for (int i=0;i<9;i++) {
            for (int j = 0; j < 9; j++) {
                if (board[i][j]==0) {
                    if (!judgeAvailable(i,j,1)) {
                        value++;
                    }
                }
            }
        }
    }
    if (judgeAvailable(x,y,1)) {
        board[x][y]=1;
        //若此步为黑棋计算黑棋的权利和
        for (int i=0;i<9;i++) {
            for (int j = 0; j < 9; j++) {
                if (board[i][j]==0) {
                    if (!judgeAvailable(i,j,-1)) {
                        value++;
                    }
                }
            }
        }
    }
    board[x][y]=0;
    return value;
}

//找到棋盘上权利和最大的点集
pair<vector<int>,int> findMaxValuePoint(const vector<int>&availableList) {
    int max_value=-1;
    vector<int>waitList;
    for (int i : availableList) {
        int x=i/9,y=i%9;
        int value=valuePoint(x,y);
        if (value>max_value) {
            max_value=value;
            waitList.clear();
            waitList.push_back(i);
        }else if (value==max_value) {
            waitList.push_back(i);
        }
    }

    return make_pair(waitList,max_value);
}

//打散规则
int getScatterPoint(const vector<int>&availableList) {
    //计算与已有点的曼哈顿距离的最小值
    int max_dis = -1;
    int result = -1;
    for(auto p: availableList) {
        int x = p/9, y = p%9;
        int min_dis = 100;
        for (int i=0;i<9;i++) {
            for (int j = 0; j < 9; j++) {
                if (board[i][j]==-1) {
                    min_dis = min(min_dis, abs(x-i)+abs(y-j));
                }
            }
        }
        if (min_dis>max_dis) {
            max_dis = min_dis;
            result = p;
        }
    }

    return result;
}

//根据waitList往下递归查看是否有最优解，若超过三层则退出执行打散规则
int getMaxValuePoint(vector<int>waitList) {
    if (waitList.empty()) {
        return -1;
    }
    if(waitList.size()==1) {
        return waitList[0];
    }
    //假设在waitList中的点行棋
    pair<int,int> result(-1,-1);
    for (int i = 0; i < waitList.size();i++) {
        int x=waitList[i]/9,y=waitList[i]%9;
        board[x][y]=-1;

        auto [newWaitList,max_value]=findMaxValuePoint(waitList);
        if (max_value && max_value > result.second) {
            result.first = waitList[i];
            result.second = max_value;
        }

    }
    return result.first;
}

int main()
{
    srand((unsigned)time(0));
    string str;
    int x,y;
    // 读入JSON
    getline(cin,str);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);
    // 分析自己收到的输入和自己过往的输出，并恢复状态
    int turnID = input["responses"].size();
    for (int i = 0; i < turnID; i++)
    {
        x=input["requests"][i]["x"].asInt(), y=input["requests"][i]["y"].asInt();
        if (x!=-1) board[x][y]=1;
        x=input["responses"][i]["x"].asInt(), y=input["responses"][i]["y"].asInt();
        if (x!=-1) board[x][y]=-1;
    }
    x=input["requests"][turnID]["x"].asInt(), y=input["requests"][turnID]["y"].asInt();
    if (x!=-1) board[x][y]=1;
    // 输出决策JSON
    Json::Value ret;
    Json::Value action;



    if (x==-1) {
        action["x"]=4; action["y"]=4;
    }else {
        vector<int> availableList;
        for (int i=0;i<9;i++) {
            for (int j = 0; j < 9; j++) {
                if (judgeAvailable(i,j,-1)) {
                    availableList.push_back(i*9+j);
                }
            }
        }
        auto [waitList,max_value]=findMaxValuePoint(availableList);
        int result;
        if (max_value==0) {
            //执行打散规则
            result = getScatterPoint(availableList);
        }else {
            result = getMaxValuePoint(waitList);
        }


        action["x"]=result/9; action["y"]=result%9;
    }
    ret["response"] = action;
    Json::FastWriter writer;

    cout << writer.write(ret) << endl;
    return 0;
}