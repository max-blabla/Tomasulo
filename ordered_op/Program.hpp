//
// Created by 16963 on 2021/6/28.
//
#ifndef RISCV_DECODING_HPP
#define RISCV_DECODING_HPP
#include<cstdio>
#include<vector>
#include<iostream>
#include"Execute.hpp"
//
// Created by 16963 on 2021/6/29.
//
class Program:public Execute{
private:
    int AscIIHash[23]={0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15};
    char ArcHash[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    inline int HexToDec(char * Hex){
        int Dec=0;
        for(int i=0;i<8;i++){
            Dec*=16;
            Dec+=AscIIHash[Hex[i]-'0'];
        }
        return Dec;
    }
    inline void ShowDectoHex(int & Dec){
        char Byte[8];
        int tmp=Dec;
        int index;
        for(int i=0;i<8;i++) {
            SufBin(index,tmp,4);
            Byte[7-i] = ArcHash[index];
        }
        std::cout<<Byte<<'\n';
    }
public:
    Program()=default;
/*    int & operator[](int index){
        return StaticOrder[index];
    }*/
    void ReadIO(){
       // char c1,c2,c3,c4,c5,c6,c7,c8;
        char c[8];
        //  std::vector<unsigned int> OrderVec;
        unsigned char Order;
        int Posi;
        int w;
        char ch;
        int count=0;
        while(scanf("%c",c)!=EOF) {
            Order = 0;
            if (c[0] == '@') {
                scanf("%c%c%c%c%c%c%c%c\n", c, (c+1), (c+2), (c+3), (c+4), (c+5), (c+6), (c+7));
                Posi=HexToDec(c);
                count=0;
            }
            else if(c[0]=='#') break;
            else {
                scanf("%c ",c+1);
                Order ^= AscIIHash[c[1] - '0'];
                Order ^= (AscIIHash[c[0] - '0'] << 4);
                count++;
                ram[Posi++] = Order;
            }
        }
        std::cout<<'/'<<'\n';
    }
    void Read(char * Filename){
        FILE * file=fopen(Filename,"r+");
        char c[8];
        //  std::vector<unsigned int> OrderVec;
        unsigned char Order;
        int Posi;
        while(fscanf(file,"%c",c)!=-1) {
            Order=0;
            if (c[0] == '@') {
                fscanf(file,"%c%c%c%c%c%c%c%c\n", c, (c+1), (c+2), (c+3), (c+4), (c+5), (c+6), (c+7));
                Posi=HexToDec(c);
            } else {
                fscanf(file,"%c ",c+1);
                Order ^= AscIIHash[c[1] - '0'];
                Order ^= (AscIIHash[c[0] - '0'] << 4);
                ram[Posi++] = Order;
                std::cout<<Posi<<' '<<Order<<'\n';
            }
        }
    }
    unsigned int Run(){
        reg.pc()=0;
        int Order;
        int addr;
        while(!csrReg.isHalt()){
            //0ff00513为结束
            addr=reg.pc();
            Order=ram.LoadOrder(addr);
            //std::cout<<addr<<' '<<'|'<<' ';
            //ShowDectoHex(Order);
            Exec(Order,addr);
        }
        return ((unsigned int)reg[10])&255u;
    }
};
#endif //RISCV_DECODING_HPP
