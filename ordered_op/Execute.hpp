//
// Created by 16963 on 2021/6/28.
//

#ifndef RISCV_EXECUTE_HPP
#define RISCV_EXECUTE_HPP
#define SufBin(y,x,k) y=(x&((1<<k)-1));x>>=k
#define MovSufBin(y,x,k,m) y^=((x&((1<<k)-1))<<m);x>>=k
#define Sext(x,k) x<<=k;x>>=k
#include "Memory.h"
#include "Register.hpp"
class Ebreak{};
class Ecall{};
//
// Created by 16963 on 2021/6/29.
//
#include "Decoding.h"
RegFile reg;
CsrReg csrReg;
enum class Type{
    R,S,U,I,B,J,L
};
RandomAccessMem ram;
class Execute{
private:

    int rs1;
    int rs2;
    int rd;
    int opcode;
    int funct3;
    int funct7;
    int immediate;
    Type OpType;
    unsigned int U_Exec(int & pc){
        unsigned int ans;
        switch(opcode){
            case 55:
                ans=immediate;
                break;//lui
            case 23:
                ans=pc+immediate;
                break;//auipc
        }
        pc+=4;
        return ans;
    }
    unsigned int J_Exec(int & pc){
        //jal
        unsigned int ans=pc+4;
        Sext(immediate,11);
        pc+=immediate;
        return ans;
    }
    bool B_Exec(){
        Sext(immediate,19);
        switch(funct3){
            case 0://beq
                if(reg[rs1]==reg[rs2]) return true;
                else return false;
            case 1://bne
                if(reg[rs1]!=reg[rs2]) return true;
                else return false;
            case 4://blt
                if(((signed)reg[rs1])<((signed)reg[rs2])) return true;
                else return false;
            case 5://bge
                if(((signed)reg[rs1])>=((signed)reg[rs2])) return true;
                else return false;
            case 6://bltu
                if(reg[rs1]<reg[rs2]) return true;
                else return false;
            case 7://bgeu
                if(reg[rs1]>=reg[rs2]) return true;
                else return false;
        }
    }
    void S_Exec(int & pc){
        pc+=4;
        Sext(immediate,20);
        switch(funct3){
            case 0: {
                unsigned int tmp=reg[rs2];
                unsigned char Byte;
                SufBin(Byte,tmp,8);
                ram[reg[rs1] + immediate]= Byte;
                break;//sb
            }
            case 1:{
                unsigned char Byte;
                unsigned int tmp=reg[rs2];
                SufBin(Byte,tmp,8);
                ram[reg[rs1]+immediate]=Byte;
                SufBin(Byte,tmp,8);
                ram[reg[rs1]+immediate+1]=Byte;
                break;
            }//sh
            case 2:{
                unsigned char Byte;
                unsigned int tmp=reg[rs2];
                SufBin(Byte,tmp,8);
                ram[reg[rs1]+immediate]=Byte;
                SufBin(Byte,tmp,8);
                ram[reg[rs1]+immediate+1]=Byte;
                SufBin(Byte,tmp,8);
                ram[reg[rs1]+immediate+2]=Byte;
                SufBin(Byte,tmp,8);
                ram[reg[rs1]+immediate+3]=Byte;
                break;
            }//sw
        }
    }
    unsigned int R_Exec(int & pc){
        unsigned int ans=0;
        pc+=4;
        int funct=funct3+funct7;
        switch(funct){
            case 0://add
                ans=reg[rs1]+reg[rs2];
                break;
            case 32://sub
                ans=reg[rs1]-reg[rs2];
                break;
            case 1: {//sll
                int shamt;
                unsigned int tmp=reg[rs2];
                SufBin(shamt, tmp, 5);
                ans = reg[rs1] << shamt;
                break;
            }
            case 2://slt
                if(((signed)reg[rs1])<((signed)reg[rs2])) ans=1;
                else ans=0;
                break;
            case 3://sltu
                if(reg[rs1]<reg[rs2]) ans=1;
                else ans=0;
                break;
            case 4://xor
                ans=reg[rs1]^reg[rs2];
                break;
            case 5: {//srl
                int shamt;
                unsigned int tmp=reg[rs2];
                SufBin(shamt, tmp, 5);
                ans = reg[rs1] >> shamt;
                break;
            }
            case 37: {//sra
                int shamt;
                unsigned int tmp=reg[rs2];
                SufBin(shamt,tmp,5);
                ans = ((signed)reg[rs1]) >> shamt;
                break;
            }
            case 6://or
                ans=reg[rs1]|reg[rs2];
                break;
            case 7://and
                ans=reg[rs1]&reg[rs2];
                break;
        }
        return ans;
    }
    unsigned int L_Exec(int & pc){
        //lb lh lw lbu lhu
        Sext(immediate,20);
        int ans=0;
        pc+=4;
        switch(funct3){
                case 0://lb
                    ans=ram[reg[rs1]+immediate];
                    Sext(ans,24);
                    break;
                case 1: {//lh
                    int tmp = 0;
                    int Byte;
                    int addr = reg[rs1] + immediate;
                    Byte = ram[addr];
                    tmp += Byte;
                    Byte = ram[addr+1];
                    tmp +=(Byte<<8);
                    ans=tmp;
                    Sext(ans,16);
                    break;
                }
                case 2: {//lw
                    int tmp=0;
                    int Byte;
                    int addr=reg[rs1]+immediate;
                    Byte = ram[addr];
                    tmp += Byte;
                    Byte=ram[addr+1];
                    tmp +=(Byte<<8);
                    Byte=ram[addr+2];
                    tmp +=(Byte<<16);
                    Byte=ram[addr+3];
                    tmp +=(Byte<<24);
                    /*std::cout<<"--------"<<'\n';
                    std::cout<<addr<<'\n';
                    std::cout<<tmp<<'\n';*/
                    ans=tmp;
                    break;
                }
                case 4: {//lbu
                    unsigned int tmp = (ram[reg[rs1] + immediate]);
                    Sext(tmp, 24);
                    ans = tmp;
                    break;
                }
                case 5://lhu
                    unsigned int tmp = 0;
                    int Byte;
                    int addr = reg[rs1] + immediate;
                    Byte = ram[addr];
                    tmp += Byte;
                    Byte=ram[addr+1];
                    tmp+=(Byte<<8);
                    Sext(tmp,16);
                    ans=tmp;
                    break;
            }
            return ans;
    }
    unsigned int I_Exec(int & pc){
        Sext(immediate,20);
        unsigned int ans;
        switch(opcode) {
            case 103: {//jalr
                ans = pc + 4;
                //Sext(immediate,20);
                pc = (reg[rs1] + immediate) & (~1);
                break;
            }
            case 19: {//addi slti sltiu xori ori andi slli srli srai
                pc += 4;
                switch (funct3) {
                    case 0: {//addi
                        if (rd == 10 && immediate == 255 && rs1 == 0) {
                            ans = reg[rd];
                            csrReg.Halt();
                        } else {
                            ans = reg[rs1] + immediate;
                        }
                        break;
                    }
                    case 1: {//slli
                        int shamt = (immediate & ((1 << 5) - 1));
                        ans = reg[rs1] << shamt;
                        break;
                    }
                    case 2://slti
                        if ((signed) reg[rs1] < immediate) ans = 1;
                        else ans = 0;
                        break;
                    case 3://sltiu
                        if (reg[rs1] < (unsigned int) immediate) ans = 1;
                        else ans = 0;
                        break;
                    case 4://xori
                        ans = reg[rs1] ^ immediate;
                        break;
                    case 5: {//
                        int shamt;
                        SufBin(shamt, immediate, 5);
                        if (immediate) {//srai
                            ans =((signed)reg[rs1]) >> shamt;
                        } else {//srli
                            ans = (reg[rs1] >> shamt);
                        }
                        break;
                    }
                    case 6://ori
                        ans = reg[rs1] | immediate;
                        break;
                    case 7://andi
                        ans = reg[rs1] & immediate;
                        break;
                }
                break;
            }
/*            case 15: {//fence fence.i
                pc += 4;
                if (funct3) {}//fence.i
                else {}//fence
                break;
            }
            case 115: {//ebreak ecall csrrw csrrs csrrc csrrwi cssrrsi csrrci
                pc += 4;
                switch (funct3) {
                    case 0: {
                        if (immediate) { throw Ebreak(); } //ebreak
                        else { throw Ecall(); }//ecall
                        //break;
                    }
                    case 1: {//csrrw
                        ans = csrReg[immediate];
                        csrReg[immediate] = reg[rs1];
                        break;
                    }
                    case 2: {//csrrs
                        int tmp = csrReg[immediate];
                        csrReg[immediate] = tmp | reg[rs1];
                        ans = tmp;
                        break;
                    }
                    case 3: {//csrrc
                        int tmp = csrReg[immediate];
                        csrReg[immediate] = tmp & (~reg[rs1]);
                        ans = tmp;
                        break;
                    }
                    case 5://csrrwi
                        ans = csrReg[immediate];
                        csrReg[immediate] = rs1;
                        break;
                    case 6: {//cssrrsi
                        int tmp = csrReg[immediate];
                        csrReg[immediate] = tmp | rs1;
                        ans = tmp;
                        break;
                    }
                    case 7: {//csrrci
                        int tmp = csrReg[immediate];
                        csrReg[immediate] = tmp & (~rs1);
                        ans = tmp;
                        break;
                    }
                }
                break;
            }
        }*/}
        return ans;
    }
    void decoding(int & BinOrder){
    int tmp=BinOrder;
    immediate=0;
    opcode=rs1=rs2=funct3=funct7=rd=0;
    SufBin(opcode,tmp,7);
    /*opcode=(tmp&(1<<6));
    tmp>>=7;*/
    if(opcode==55||opcode==23){//lui auipc
        OpType=Type::U;
        SufBin(rd,tmp,5);
        MovSufBin(immediate,tmp,20,12);
    }
    else if(opcode==111){//jal
        OpType=Type::J;
        SufBin(rd,tmp,5);
        MovSufBin(immediate,tmp,8,12);
        MovSufBin(immediate,tmp,1,11);
        MovSufBin(immediate,tmp,10,1);
        MovSufBin(immediate,tmp,1,20);
     /*   immediate^=((tmp&((1<<8)-1))<<12);
        tmp>>=8;
        immediate^=((tmp&1)<<11);
        tmp>>=1;
        immediate^=((tmp&((1<<10)-1))<<1);
        tmp>>=10;
        immediate^=((tmp&1)<<20);*/
    }
    else if(opcode==51){//add sub sll slt sltu xor srl sra or and
        OpType=Type::R;
        SufBin(rd,tmp,5);
        SufBin(funct3,tmp,3);
        SufBin(rs1,tmp,5);
        SufBin(rs2,tmp,5);
        SufBin(funct7,tmp,7);
        /*  funct3=tmp&(1<<2);
          tmp>>=3;
          rs1=tmp&(1<<4);
          tmp>>=5;
          rs2=tmp&(1<<4);
          tmp>>=5;
          funct7=tmp&(1<<6);*/
    }
    else if(opcode==99){//beq bne blt bge bltu bgeu
        OpType=Type::B;
        MovSufBin(immediate,tmp,1,11);
        MovSufBin(immediate,tmp,4,1);
        SufBin(funct3,tmp,3);
        SufBin(rs1,tmp,5);
        SufBin(rs2,tmp,5);
        MovSufBin(immediate,tmp,6,5);
        MovSufBin(immediate,tmp,1,12);
    }
    else if(opcode==35){//sb sh sw
        OpType=Type::S;
        SufBin(immediate,tmp,5);
        SufBin(funct3,tmp,3);
        SufBin(rs1,tmp,5);
        SufBin(rs2,tmp,5);
        MovSufBin(immediate,tmp,7,5);
        /*rs2=tmp&(1<<4);
        tmp>>=5;*/
    }
    else if(opcode==3){//lb lh lw lbu lhu
        OpType=Type::L;
        SufBin(rd,tmp,5);
        SufBin(funct3,tmp,3);
        SufBin(rs1,tmp,5);
        SufBin(immediate,tmp,12);
    }
    else{//jalr  addi slti sltiu xori ori andi slli srli srai ecall ebreak csrrw csrrs csrrc csrrwi csrrsi csrrci
        OpType=Type::I;
        SufBin(rd,tmp,5);
        SufBin(funct3,tmp,3);
        SufBin(rs1,tmp,5);
        SufBin(immediate,tmp,12);
    }
    return;
    //return;
}
public:
    Execute()=default;
    void Exec(int & BinOrder,int addr){
        decoding(BinOrder);
        switch (OpType) {
            case Type::U:{
                unsigned int ans=U_Exec(reg.pc());
                if(rd) reg[rd]=ans;
                break;
            }
            case Type::J:{
                unsigned int ans=J_Exec(reg.pc());
                if(rd) reg[rd]=ans;
                break;
            }
            case Type::B:{
                bool flag=B_Exec();
                if(flag) reg.pc()+=immediate;
                else reg.pc()+=4;
                break;
            }
            case Type::S:{
                S_Exec(reg.pc());
                break;
            }
            case Type::R:{
                unsigned int ans=R_Exec(reg.pc());
                if(rd) reg[rd]=ans;
                break;
            }
            case Type::I:{
                unsigned int ans=I_Exec(reg.pc());
                if(rd) reg[rd]=ans;
                break;
            }
            case Type::L:{
                unsigned int ans=L_Exec(reg.pc());
                if(rd) reg[rd]=ans;
                break;
            }
        }
    }
};

#endif //RISCV_EXECUTE_HPP
