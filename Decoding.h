//
// Created by 16963 on 2021/6/29.
//

#ifndef RISCV_DECODING_H
#define RISCV_DECODING_H
#define SufBin(y,x,k) y=(x&((1<<k)-1));x>>=k
#define MovSufBin(y,x,k,m) y^=((x&((1<<k)-1))<<m);x>>=k
#define Sext(x,k) x<<=k;x>>=k
#include <iostream>
enum class Type{
    R,S,U,I,B,J,L,E,Jr
};
struct Issue{
    int rs1;
    int rs2;
    int rd;
    int opcode;
    int funct3;
    int funct7;
    int immediate;
    Type OpType;
    int ROBid;
    int pc;
    void SetZero(){
        rs1=rs2=rd=opcode=funct3=funct7=immediate=ROBid=0;
    }
};
class DecodeTool {
private:
    int rs1;
    int rs2;
    int rd;
    int opcode;
    int funct3;
    int funct7;
    int immediate;
    Type OpType;
public:
   Issue decoding(int &BinOrder) {
        int tmp = BinOrder;
        immediate = 0;
        opcode = rs1 = rs2 = funct3 = funct7 = rd = 0;
        SufBin(opcode, tmp, 7);
        /*opcode=(tmp&(1<<6));
        tmp>>=7;*/
        if (opcode == 55 || opcode == 23) {//lui auipc
            OpType = Type::U;
            SufBin(rd, tmp, 5);
            MovSufBin(immediate, tmp, 20, 12);
        }
        else if (opcode == 111) {//jal
            OpType = Type::J;
            SufBin(rd, tmp, 5);
            MovSufBin(immediate, tmp, 8, 12);
            MovSufBin(immediate, tmp, 1, 11);
            MovSufBin(immediate, tmp, 10, 1);
            MovSufBin(immediate, tmp, 1, 20);
        }
        else if(opcode==103) {//jalr
            OpType = Type::Jr;
            SufBin(rd, tmp, 5);
            SufBin(funct3, tmp, 3);
            SufBin(rs1, tmp, 5);
            SufBin(immediate, tmp, 12);
        }
        else if (opcode == 51) {//add sub sll slt sltu xor srl sra or and
            OpType = Type::R;
            SufBin(rd, tmp, 5);
          //  std::cout<<BinOrder<<" "<<rd<<'\n';
            SufBin(funct3, tmp, 3);
            SufBin(rs1, tmp, 5);
            SufBin(rs2, tmp, 5);
            SufBin(funct7, tmp, 7);
        } else if (opcode == 99) {//beq bne blt bge bltu bgeu
            OpType = Type::B;
            MovSufBin(immediate, tmp, 1, 11);
            MovSufBin(immediate, tmp, 4, 1);
            SufBin(funct3, tmp, 3);
            SufBin(rs1, tmp, 5);
            SufBin(rs2, tmp, 5);
            MovSufBin(immediate, tmp, 6, 5);
            MovSufBin(immediate, tmp, 1, 12);
        } else if (opcode == 35) {//sb sh sw
            OpType = Type::S;
            rd=0;
            SufBin(immediate, tmp, 5);
            SufBin(funct3, tmp, 3);
            SufBin(rs1, tmp, 5);
            SufBin(rs2, tmp, 5);
            MovSufBin(immediate, tmp, 7, 5);
            /*rs2=tmp&(1<<4);
            tmp>>=5;*/
        } else if (opcode == 3) {//lb lh lw lbu lhu
            OpType = Type::L;
            SufBin(rd, tmp, 5);
            SufBin(funct3, tmp, 3);
            SufBin(rs1, tmp, 5);
            SufBin(immediate, tmp, 12);
        } else {//jalr  addi slti sltiu xori ori andi slli srli srai ecall ebreak csrrw csrrs csrrc csrrwi csrrsi csrrci
            OpType = Type::I;
            SufBin(rd, tmp, 5);
            SufBin(funct3, tmp, 3);
            SufBin(rs1, tmp, 5);
            SufBin(immediate, tmp, 12);
        }
        Issue Ret;
        Ret.opcode=opcode;
        Ret.rd=rd;
        Ret.funct3=funct3;
        Ret.OpType=OpType;
        Ret.rs1=rs1;
        Ret.rs2=rs2;
        Ret.immediate=immediate;
        Ret.funct7=funct7;
        return Ret;
        //return;
    }
};

/*
};
struct DataGroup{
    short rs1;
    short rs2;
    short rd;
    short opcode;
    short funct3;
    short funct7;
    int immediate;
    Type OpType;
    DataGroup(Type & type,short & trs1,short & trs2,short & trd,short & topcode,short & tfunct3,short & tfunct7, int & timmediate){
        OpType=type;
        rs1=trs1;
        rs2=trs2;
        rd=trd;
        opcode=topcode;
        funct3=tfunct3;
        funct7=tfunct7;
        immediate=timmediate;
    }
};
class DecodingTool{
private:
    short rs1=0;
    short rs2=0;
    short rd=0;
    short opcode=0;
    short funct3=0;
    short funct7=0;
    int immediate=0;
    Type OpType;
public:
    DataGroup decoding(int & BinOrder){
        int tmp=BinOrder;
        immediate=0;
        opcode=SufBin(tmp,7);
        *//*opcode=(tmp&(1<<6));
        tmp>>=7;*//*
        if(opcode==55||opcode==23){//lui auipc
            OpType=Type::U;
            rd=SufBin(tmp,5);
            *//*rd=(tmp&(1<<4));
            tmp>>=5;*//*
            immediate=(tmp&(1<<19));
        }
        else if(opcode==111){//jal
            OpType=Type::J;
            rd=SufBin(tmp,5);
            *//*rd=tmp&(1<<4);
            tmp>>=5;*//*
            immediate^=((tmp&((1<<8)-1))<<12);
            tmp>>=8;
            immediate^=((tmp&1)<<11);
            tmp>>=1;
            immediate^=((tmp&((1<<10)-1))<<1);
            tmp>>=10;
            immediate^=((tmp&1)<<20);
        }
        else if(opcode==51){//add sub sll slt sltu xor srl sra or and
            OpType=Type::R;
            rd=SufBin(tmp,5);
            *//*rd=tmp&(1<<4);
            tmp>>=5;*//*
            funct3=SufBin(tmp,3);
            rs1=SufBin(tmp,5);
            rs2=SufBin(tmp,5);
            funct7=SufBin(tmp,7);
            *//*  funct3=tmp&(1<<2);
              tmp>>=3;
              rs1=tmp&(1<<4);
              tmp>>=5;
              rs2=tmp&(1<<4);
              tmp>>=5;
              funct7=tmp&(1<<6);*//*
        }
        else if(opcode==99){//beq bne blt bge bltu bgeu
            OpType=Type::B;
            immediate^=((tmp&1)<<11);
            tmp>>=1;
            immediate^=((tmp&((1<<4)-1))<<1);
            tmp>>=4;
            funct3=SufBin(tmp,3);
            *//*funct3=tmp&(1<<2);
            tmp>>=3;*//*
            rs1=SufBin(tmp,5);
            *//*rs1=tmp&(1<<4);
            tmp>>=5;*//*
            rs2=SufBin(tmp,5);
            *//*rs2=tmp&(1<<4);
            tmp>>=5;*//*
            immediate^=((tmp&((1<<6)-1))<<5);
            tmp>>=6;
            immediate^=((tmp&1)<<12);
        }
        else if(opcode==35){//sb sh sw
            OpType=Type::S;
            immediate^=(tmp&((1<<5)-1));
            tmp>>=5;
            rd=SufBin(tmp,3);
            *//*funct3=tmp&(1<<2);
            tmp>>=3;*//*
            rs1=SufBin(tmp,5);
            *//*rs1=tmp&(1<<4);
            tmp>>=5;*//*
            rs2=SufBin(tmp,5);
            *//*rs2=tmp&(1<<4);
            tmp>>=5;*//*
            immediate^=(tmp&((1<<7)-1));
        }
        else if(opcode==3){//lb lh lw lbu lhu
            OpType=Type::L;
            rd=SufBin(tmp,5);
            funct3=SufBin(tmp,3);
            rs1=SufBin(tmp,5);
            immediate^=(tmp&((1<<12)-1));
        }
        else{//jalr  addi slti sltiu xori ori andi slli srli srai ecall ebreak csrrw csrrs csrrc csrrwi csrrsi csrrci
            OpType=Type::I;
            rd=SufBin(tmp,5);
            funct3=SufBin(tmp,3);
            rs1=SufBin(tmp,5);
            immediate^=(tmp&((1<<12)-1));
        }
        DataGroup ret(OpType,rs1,rs2,rd,opcode,funct3,funct7,immediate);
        return ret;
        //return;
    }
};*/
#endif //RISCV_DECODING_H
