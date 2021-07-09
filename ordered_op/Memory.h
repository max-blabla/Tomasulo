
//
// Created by 16963 on 2021/6/29.
//

#ifndef RISCV_MEMORY_H
#define RISCV_MEMORY_H
class RandomAccessMem{
private:
    unsigned char Ram[1000000];
public:
    RandomAccessMem()=default;
/*    void Store(int  data,int addr){
        ram[addr]=data;
    }*/
    void WriteMem(unsigned char & data, int & addr){
        Ram[addr]=data;
    }
    unsigned char  ReadMem(int & addr){
        return Ram[addr];
    }
    unsigned char & operator[](int addr){
        return Ram[addr];
    }
    inline int LoadOrder(int addr){
        int Order=0;
        unsigned char Byte;
        int tmp=0;
        for(int i=0;i<4;i++){
            Byte=Ram[addr+i];
            tmp^=Byte;
            tmp<<=(8*i);
            Order^=tmp;
            tmp=0;
        }
        return Order;
    }
    ~RandomAccessMem()=default;
};

#endif //RISCV_MEMORY_H
