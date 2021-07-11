//
// Created by 16963 on 2021/6/28.
//

#ifndef RISCV_EXECUTE_HPP
#define RISCV_EXECUTE_HPP
//
// Created by 16963 on 2021/6/29.
//
#include "Decoding.h"
#include<iostream>
class ExecuteTool{
    std::pair<int,int> U_Exec(int & pc,Issue & issue,int & xrs1,int & xrs2){
        int ans;
        switch(issue.opcode){
            case 55:
                ans=issue.immediate;
                break;//lui
            case 23:
                ans=pc+issue.immediate;
                break;//auipc
        }
        return std::pair<int,int>(ans,pc+4);
    }
    std::pair<int,int> J_Exec(int & pc,Issue & issue,int & xrs1,int & xrs2){
        //jal
        int ans=pc+4;
        int immediate=issue.immediate;
                Sext(immediate,11);
                return std::pair<int,int>(ans,pc+immediate);
            //jalr
                //Sext(immediate,20);

    }
    std::pair<int,int> Jr_Exec(int & pc,Issue & issue,int & xrs1,int & xrs2) {
        int ans = pc + 4;
        int immediate = issue.immediate;
        Sext(immediate, 20);
        return std::pair<int, int>(ans, (xrs1 + immediate) & (~1));
    }
    std::pair<int,int> B_Exec(int & pc,Issue & issue,int & xrs1,int & xrs2){
        int immediate=issue.immediate;
        Sext(immediate,19);
        bool flag;
        switch(issue.funct3){
            case 0://beq
                if(xrs1==xrs2) flag=true;
                else flag=false;
                break;
            case 1://bne
      /*      std::cout<<"bne"<<'\n';
            std::cout<<xrs1<<' '<<xrs2<<'\n';*/
                if(xrs1!=xrs2) flag=true;
                else flag=false;
                break;
            case 4://blt
                if(xrs1<xrs2)flag=true;
                else flag=false;
                break;
            case 5://bge
                if(xrs1>=xrs2) flag= true;
                else flag=false;
                break;
            case 6://bltu
                if(unsigned (xrs1)<unsigned (xrs2)) flag=true;
                else flag=false;
                break;
            case 7://bgeu
                if(unsigned (xrs1)>=unsigned (xrs2)) flag=true;
                else flag=false;
                break;
        }
        if(flag) return std::pair<int,int>(1,pc+immediate);
        else return std::pair<int,int>(0,pc+4);
    }
   /*
    }*/
    std::pair<int,int> R_Exec(int & pc,Issue & issue,int & xrs1,int & xrs2){
        int ans=0;
        pc+=4;
        int funct=issue.funct3+issue.funct7;
        switch(funct){
            case 0://add
                ans=xrs1+xrs2;
                break;
            case 32://sub
                ans=xrs1-xrs2;
                break;
            case 1: {//sll
                int shamt;
                unsigned int tmp=xrs2;
                SufBin(shamt, tmp, 5);
                ans = xrs1 << shamt;
                break;
            }
            case 2://slt
                if(xrs1<xrs2) ans=1;
                else ans=0;
                break;
            case 3://sltu
                if(unsigned(xrs1)<unsigned (xrs2)) ans=1;
                else ans=0;
                break;
            case 4://xor
                ans=xrs1^xrs2;
                break;
            case 5: {//srl
                int shamt;
                unsigned int tmp=xrs2;
                SufBin(shamt, tmp, 5);
                ans =( (unsigned) xrs1) >> shamt;
                break;
            }
            case 37: {//sra
                int shamt;
                unsigned int tmp=xrs2;
                SufBin(shamt,tmp,5);
                ans = xrs1 >> shamt;
                break;
            }
            case 6://or
                ans=xrs1|xrs2;
                break;
            case 7://and
                ans=xrs1&xrs2;
                break;
        }
        return std::pair<int,int>(ans,pc+4);
    }

    std::pair<int,int> I_Exec(int & pc,Issue & issue,int & xrs1,int & xrs2){
        int immediate=issue.immediate;
        Sext(immediate,20);
        int ans;
        switch(issue.opcode) {
            case 19: {//addi slti sltiu xori ori andi slli srli srai
                pc += 4;
                switch (issue.funct3) {
                    case 0://addi
                        ans = xrs1 + immediate;
                        break;
                    case 1: {//slli
                        int shamt = (immediate & ((1 << 5) - 1));
                        ans = xrs1 << shamt;
                        break;
                    }
                    case 2://slti
                        if (xrs1 < immediate) ans = 1;
                        else ans = 0;
                        break;
                    case 3://sltiu
                        if (unsigned(xrs1) < (unsigned int) immediate) ans = 1;
                        else ans = 0;
                        break;
                    case 4://xori
                        ans = xrs1 ^ immediate;
                        break;
                    case 5: {//
                        int shamt;
                        SufBin(shamt, immediate, 5);
                        if (immediate) {//srai
                            ans = (xrs1) >> shamt;
                        } else {//srli
                            ans = (unsigned(xrs1)) >> shamt;
                        }
                        break;
                    }
                    case 6://ori
                        ans = xrs1 | immediate;
                        break;
                    case 7://andi
                        ans = xrs1 & immediate;
                        break;
                }
                break;
            }
        }
        return std::pair<int,int>(ans,pc+4);
    }
public:
    ExecuteTool()=default;
    std::pair<int,int> Exec(Issue & issue,int Pc,int xrs1,int xrs2){
        //一个是Pc变化，一个是值
        switch (issue.OpType) {
            case Type::U:{
                std::pair<int,int> ans=U_Exec(Pc,issue,xrs1,xrs2);
                return ans;
            }
            case Type::J:{
                std::pair<int,int> ans=J_Exec(Pc,issue,xrs1,xrs2);
                return ans;
            }
            case Type::B:{
                std::pair<int,int> ans=B_Exec(Pc,issue,xrs1,xrs2);
                return ans;
            }
            case Type::R:{
                std::pair<int,int> ans=R_Exec(Pc,issue,xrs1,xrs2);
                return ans;
            }
            case Type::I:{
                std::pair<int,int> ans=I_Exec(Pc,issue,xrs1,xrs2);
                return ans;
            }
            case Type::Jr:{
                std::pair<int,int>ans=Jr_Exec(Pc,issue,xrs1,xrs2);
                return ans;
            }
        }
    }
};

#endif //RISCV_EXECUTE_HPP
