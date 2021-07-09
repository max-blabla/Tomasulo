#include "Program.hpp"
/*int AscIIHash[23]={0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15};
char ArcHash[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
inline int HexToDec(char * Hex){
    int Dec=0;
    for(int i=0;i<8;i++){
        Dec*=16;
        Dec+=AscIIHash[Hex[i]-'0'];
    }
    return Dec;
}*/
int main() {
    Program program;
    program.ReadIO();
    std::cout<<program.Run()<<'\n';
}
/*#include "io.inc"

int N = 8;
int row[8];
int col[8];
int d[2][16];

void printBoard() {
    int i;
    int j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (col[i] == j)
                printStr(" O");
            else
                printStr(" .");
        }
        printStr("\n");
    }
    printStr("\n");
}

void search(int c) {
    if (c == N) {
        printBoard();
        std::cout<<judgeResult<<"\n";
    } else {
        int r;
        for (r = 0; r < N; r++) {
            if (row[r] == 0 && d[0][r + c] == 0 && d[1][r + N - 1 - c] == 0) {
                row[r] = d[0][r + c] = d[1][r + N - 1 - c] = 1;
                col[c] = r;
                search(c + 1);
                row[r] = d[0][r + c] = d[1][r + N - 1 - c] = 0;
            }
        }
    }
}

int main() {
    search(0);

    return judgeResult % Mod;  // 171
}*/

/*char Hex[8];

int immediate;
int funct3;
int rs1;
int rs2;
int opcode;

int tmp;
int a=-1;
int b=1;
if(a<b) std::cout<<'?'<<'\n';
if((unsigned int)a<(unsigned int)b) std::cout<<'!'<<'\n';
return 0;
while(std::cin) {
    std::cin >> Hex;
    tmp=HexToDec(Hex);
    SufBin(opcode,tmp,7);
    std::cout<<tmp<<'\n';
    MovSufBin(immediate,tmp,1,11);
    std::cout<<immediate<<" "<<tmp<<'\n';
    MovSufBin(immediate,tmp,4,1);

    SufBin(funct3,tmp,3);
    SufBin(rs1,tmp,5);
    std::cout<<tmp<<' '<<'\n';
    SufBin(rs2,tmp,5);
    MovSufBin(immediate,tmp,6,5);
    MovSufBin(immediate,tmp,1,12);
    std::cout <<opcode<<' '<< rs1 << ' ' << rs2 << ' ' << funct3 << ' ' << immediate << "\n";
    immediate=0;
}
}*/
/*
#include<iostream>
using namespace std;
int judgeResult = 0;
const int Mod = 253;

void printInt(int x) {
    judgeResult ^= x;
    judgeResult += 173;
}

void printStr(const char *str) {
    for (const char *cur = str; *cur != 0; ++cur) {
        judgeResult ^= *cur;
        judgeResult += 521;
    }
}


int a[4];
int main() {
    int b[4];
    int i;
    for (i = 0; i < 4; i++) {
        a[i] = 0;
        b[i] = i + 1;
    }
    for (i = 0; i < 4; i++) {
        printInt(a[i]);
    }

    int *p;
    p = b;
    for (i = 0; i < 4; i++) {
        printInt(p[i]);
    }
    cout<<judgeResult % Mod; // 123
}*/
