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

#endif //RISCV_REGISTER_H
