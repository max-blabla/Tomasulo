//
// Created by 16963 on 2021/6/28.
//

#ifndef RISCV_REGISTER_HPP
#define RISCV_REGISTER_HPP
//0 hard zero
//1 ra
class CsrReg{
private:
    int csr[8]={0};
    bool halt=false;
public:
    CsrReg()=default;
    int & operator[](int index){
        return csr[index];
    }
    void Halt(){
        halt=true;
    }
    bool isHalt()const{
        return halt;
    }
    ~CsrReg()=default;
};
class RegFile{
private:
    unsigned int reg[32]={0};
    int ROBid[32]={0};
    int PC=0;
public:
    struct RegPair{
        int Data=0;
        int Rob=0;
    };
    RegFile()=default;
    int & pc(){return PC;}
    ~RegFile()=default;
    unsigned int & operator[](int index){
        return reg[index];
    }
    void WriteBack(int & addr,int & data){
        reg[addr]=data;
    }
    void WriteRob(int & addr,int & data){
        ROBid[addr]=data;
    }
    int ReadRob(int & addr){
        return ROBid[addr];
    }
    int ReadReg(int & addr){
        return reg[addr];
    }
    RegPair ReadData(int & addr){
        RegPair Ret;
        Ret.Rob=ROBid[addr];
        if(!ROBid[addr]) Ret.Data=reg[addr];
        return Ret;
    }
};
#endif //RISCV_REGISTER_H
