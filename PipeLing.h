//
// Created by 16963 on 2021/6/29.
//

#ifndef RISCV_PIPELING_H
#define RISCV_PIPELING_H
#include"Register.hpp"
#include"Decoding.h"
#include "Memory.h"
#include "Execute.hpp"
#include<cstring>
#include<iostream>
RandomAccessMem ram;
class End{};
bool False=false;
bool True=true;
    Issue IssueSign;
    std::pair<Issue,std::pair<int,int>> RsSign;
    std::pair<std::pair<int,int>,std::pair<int,int>> ExSign;
    std::pair<int,std::pair<int,int>> SlbSign;
    std::pair<int,std::pair<int,int>> CommitSign;
    std::pair<int,int> CommitSlbSign;
    bool IsEmpty=true;
    bool NewIsEmpty=true;
    bool IsClear=false;
    bool IsStuck=false;
    bool NewIsStuck=false;
    bool NewIsClear=false;
    int StuckRob;
    int PcSign;
    void ControlBusInitial(){
        IssueSign.SetZero();
        RsSign.first.SetZero();
        RsSign.second.second=RsSign.second.first=0;
        ExSign.first.first=ExSign.second.first=ExSign.first.second=ExSign.second.second=0;
        SlbSign.first=SlbSign.second.second=SlbSign.second.first=0;
        CommitSign.first=CommitSign.second.first=CommitSign.second.second=0;
        CommitSlbSign.first=CommitSlbSign.second=0;
    }

    //记ROBid和值
    //如果是写,rd设为0
    //first為ROB second.first為值 second.second為rd
    //slbsign


    //ExResult 为Ex函数们执行的信息，第一维是ROBid，第二维是返回值

    //first.first為ROB first.second為rd second.first為ans second

    //first為ROB second.second為rd second.first為值

    //first是ROBid second.first是计算结果 second.second是寄存器位置
    //commitsign


    //解码后的issue，包含分配好的ROBid
    //issuesign

    //first是解码信息 second.first是Value1 second.second是Value2

class RegFile{
private:
    int reg_Prev[33]={0};
    int ROBid_Prev[32]={0};
    int reg_Next[33]={0};
    int ROBid_Next[32]={0};
public:
    struct RegPair{
        int Data=0;
        int Rob=0;
    };
    void Print(){
        for(int i=0;i<=32;i++)
            std::cout << reg_Prev[i] << ' ';
        std::cout << '\n';
    }
    void Clear(){
            memset(ROBid_Next,0,sizeof(ROBid_Next));
            memset(ROBid_Prev,0,sizeof(ROBid_Prev));
           // ROBid_Next[i]=0;
    }
    RegFile()=default;
    ~RegFile()=default;
    void CommitReceive(){
        int rd=CommitSign.second.second;
        if(ROBid_Prev[rd]==CommitSign.first) ROBid_Prev[rd]=ROBid_Next[rd]=0;
        reg_Prev[rd]=reg_Next[rd]=CommitSign.second.first;
    }
    void IssueReceive(){
        if(!IsStuck) {
            int rd = IssueSign.rd;
            ROBid_Prev[rd]=ROBid_Next[rd] = IssueSign.ROBid;
        }
        //WriteRob(IssueSign.rd,IssueSign.ROBid);
    }
    void PcReceive(){
        reg_Prev[32]=reg_Next[32]=PcSign;
      //  if(isEmpty()) WriteRob(32,PcSign);
    }
    int ReadReg(int addr){
        return reg_Prev[addr];
    }
    RegPair ReadData(int & addr){
        RegPair Ret;
        if(addr) {
         //   std::cout<<addr<<' '<<ROBid_Prev[addr]<<' '<<reg_Prev[addr]<<'\n';
            Ret.Rob = ROBid_Prev[addr];
            if (!ROBid_Prev[addr]) Ret.Data = reg_Prev[addr];
            else Ret.Data=0;
        }
        else Ret.Data=Ret.Data=0;
        return Ret;
    }
};
RegFile reg;
class BinOrderCache{
private:
    DecodeTool decode;
    static const int Storage=1000;
    static const int Threhold=100;
    std::pair<Issue,int> Cache_Prev[Storage];
    std::pair<Issue,int> Cache_Next[Storage];
    int Head_Prev;
    int Head_Next;
    int BOCsize_Prev;
    int BOCsize_Next;
    int Count_Prev;
    int Count_Next;
    int FetchPc=0;
    bool isStall;
public:
    BinOrderCache(){
        Head_Prev=Head_Next=BOCsize_Prev=BOCsize_Next=Count_Prev=Count_Next=0;
        isStall=false;
    }
    void PushBack(Issue & Order){
        Cache_Prev[Count_Prev].first=Cache_Next[Count_Prev].first=Order;
        Cache_Prev[Count_Prev].second=Cache_Next[Count_Prev].second=FetchPc;
        BOCsize_Prev=BOCsize_Next=BOCsize_Prev+1;
        Count_Prev=Count_Next=(Count_Prev+1)%Storage;
    }
    void PopFront(){
        Head_Next=Head_Prev=(Head_Prev+1)%Storage;
        BOCsize_Next=BOCsize_Prev=BOCsize_Prev-1;
    }
    void Fetch() {
        //  std::cout<<"Fetch"<<' '<<FetchPc<<' ';
        if (!isStall) {
            int BinOrder = ram.LoadOrder(FetchPc);
            // std::cout<<BinOrder<<'\n';
            //reg.WriteReg(32,pc+4)
            //;
            Issue issue = decode.decoding(BinOrder);
            int imm = issue.immediate;
            PushBack(issue);
            if (BOCsize_Prev <= Storage - 2) {
              //  std::cout<<"Fetch"<<'\n';
             //   std::cout << BOCsize_Prev << ' '  << '\n';
                switch (issue.opcode) {
                    //要符号位拓展
                    case 99:
                    Sext(imm, 19);
                        FetchPc += imm;
                        break;//B类
                    case 111:
                    Sext(imm, 11);
                        FetchPc += imm;
                        break;//jal
                    default:
                        FetchPc += 4;
                        break;
                }
            } else {
                isStall = true;
                std::cout << "Can?" << '\n';
            }

            if (IsEmpty) {
            //    std::cout<<'?'<<'\n';
                NewIsEmpty = False;
            }
        }
    }
    void Decode(){
        Issue Order=Cache_Prev[Head_Prev].first;
        int Pc=Cache_Prev[Head_Prev].second;
        Order.pc=Pc;
        IssueSign=Order;
        if(BOCsize_Next<=Threhold) isStall=false;
        if((!IsStuck)) {
      //     std::cout<<'?'<<'?'<<'\n';
            PopFront();
        }
    }
    void Renew(){
       // if(!isStuck()) {
       // }
    }
    void Clear(){
        Head_Prev=Head_Next=BOCsize_Prev=BOCsize_Next=Count_Prev=Count_Next=0;
        FetchPc=reg.ReadReg(32);
        isStall=false;
    }
    ~BinOrderCache()=default;
};
    static const int RobStorage=30;
    static const int RobMod=31;
    int ROBsize_Prev;
    int ROBsize_Next;
    struct ReOrderUnit {
        int Value1=0;
        int Value2=0;
        int rd=0;
        Type OpType;
        bool isFinish=false;
        int pc;
    };
    ReOrderUnit ROB_Prev[RobStorage+2];
    ReOrderUnit ROB_Next[RobStorage+2];
    bool RobStoreStuck_Prev[RobStorage+2]={0};
    bool RobStoreStuck_Next[RobStorage+2]={0};
    int RobHead_Prev;
    int RobHead_Next;
    int RobCount_Prev;
    int RobCount_Next;
    //缓存新信息
    int Index;

struct BusyAddress {
    bool isCommit = false;
    RegFile::RegPair Data1;
    RegFile::RegPair Data2;
    bool isBusy = false;
    Issue ExIssue;
    int clock;
};
static const int SlbStorage = 6;
static const int SlbMod=7;
BusyAddress SLB_Prev[SlbStorage + 2];
BusyAddress SLB_Next[SlbStorage + 2];
int SlbHead_Prev;
int SlbHead_Next;
int BAsize_Prev;
int BAsize_Next;
int SlbCount_Prev;
int SlbCount_Next;


struct ReserveStationUnit{
    RegFile::RegPair Pi;
    RegFile::RegPair Pj;
    Issue ExIssue;
    int Nxt;
    int Prv;
};
static const int RsStorage=6;
ReserveStationUnit RS_Prev[RsStorage+2];
ReserveStationUnit RS_Next[RsStorage+2];
int RsSize_Prev;
int RsSize_Next;
int RsHead=0;
int RsRear=RsStorage+1;
int Pool[10];
int RsTop=0;




void NewRob(Type & type,int & rd,int & pc){

        Index=RobCount_Prev;
        ROB_Prev[RobCount_Prev].OpType=ROB_Next[RobCount_Prev].OpType=type;
        ROB_Prev[RobCount_Prev].rd=ROB_Next[RobCount_Prev].rd=rd;
        ROB_Prev[RobCount_Prev].pc=ROB_Next[RobCount_Prev].pc=pc;
        ROB_Prev[RobCount_Prev].isFinish=ROB_Next[RobCount_Prev].isFinish=false;
        if(type==Type::S)  ROB_Prev[RobCount_Next].isFinish=ROB_Next[RobCount_Prev].isFinish=true;
        if(RobCount_Prev==RobMod) RobCount_Prev=RobCount_Next=1;
        else RobCount_Next=++RobCount_Prev;
        ROBsize_Prev=ROBsize_Next=ROBsize_Prev+1;
    }
bool NewUnit(Issue & NewIssue){
    bool flag=false;
    //這裏改成隊列
    int Pos=Pool[RsTop];
    RsTop++;
    int Prev=RS_Prev[RsRear].Prv;
    RS_Next[Pos].Prv=RS_Prev[Pos].Prv=Prev;
    RS_Next[RsRear].Prv=RS_Prev[RsRear].Prv=Pos;
    RS_Next[Pos].Nxt=RS_Prev[Pos].Nxt=RsRear;
    RS_Next[Prev].Nxt=RS_Prev[Prev].Nxt=Pos;
    RS_Prev[Pos].ExIssue=RS_Next[Pos].ExIssue=NewIssue;
    RegFile::RegPair P1,P2;
    P1=reg.ReadData(NewIssue.rs1);
    P2=reg.ReadData(NewIssue.rs2);
    if(P1.Rob&&ROB_Prev[P1.Rob].isFinish){
        P1.Data=ROB_Prev[P1.Rob].Value1;
        P1.Rob=0;
    }
    if(P2.Rob&&ROB_Prev[P2.Rob].isFinish) {
        P2.Data = ROB_Prev[P2.Rob].Value1;
        P2.Rob = 0;
    }
    RS_Prev[Pos].Pi=RS_Next[Pos].Pi=P1;
    RS_Prev[Pos].Pj=RS_Next[Pos].Pj=P2;
    RsSize_Prev=RsSize_Next=RsSize_Prev+1;
    /*   std::cout<<"New"<<'\n';
       std::cout<<Pos<<'\n';*/
    //   Print();
    return flag;
}

bool NewAddr(Issue &issue) {

    /*     std::cout<<"SlbCount_Next"<<' '<<SlbCount_Next<<'\n';
         std::cout<<issue.pc<<'\n';*/
    SLB_Prev[SlbCount_Prev].ExIssue=SLB_Next[SlbCount_Prev].ExIssue = issue;
    SLB_Prev[SlbCount_Prev].isBusy=SLB_Next[SlbCount_Prev].isBusy = true;
    RegFile::RegPair Q1=reg.ReadData(issue.rs1);
    RegFile::RegPair Q2=reg.ReadData(issue.rs2);
    if(Q1.Rob&&ROB_Prev[Q1.Rob].isFinish){
        Q1.Data=ROB_Prev[Q1.Rob].Value1;
        Q1.Rob=0;
    }
    if(Q2.Rob&&ROB_Prev[Q2.Rob].isFinish){
        Q2.Data=ROB_Prev[Q2.Rob].Value1;
        Q2.Rob=0;
    }
    SLB_Prev[SlbCount_Prev].Data1=SLB_Next[SlbCount_Prev].Data1=Q1;
    SLB_Prev[SlbCount_Prev].Data2=SLB_Next[SlbCount_Prev].Data2=Q2;
    if (issue.OpType == Type::L)
        SLB_Prev[SlbCount_Prev].isCommit = SLB_Next[SlbCount_Prev].isCommit = true;
        //if(!P.Rob) SLB_Prev[SlbCount_Prev].Data1=SLB_Next[SlbCount_Next].Data1=P.Data;
    else
        SLB_Prev[SlbCount_Prev].isCommit = SLB_Next[SlbCount_Prev].isCommit = false;
    SLB_Prev[SlbCount_Prev].clock=SLB_Next[SlbCount_Prev].clock = 3;
    if(SlbCount_Prev==SlbMod) SlbCount_Prev=SlbCount_Next = 1;
    else SlbCount_Next=SlbCount_Prev=SlbCount_Prev+1;
    BAsize_Prev=BAsize_Next = BAsize_Prev + 1;
    // std::cout<<BAsize_Next<<' '<<issue.pc<<'\n';
    return true;
}
    //新issue对应Adder
    //接受函数执行结果
    //接受读写结果
    //一个周期内可能两个结果都接受
    void ReOrderInitial() {
        RobHead_Prev=RobHead_Next=1;
        ROBsize_Prev=ROBsize_Next=0;
        RobCount_Prev=RobCount_Next=1;
        memset(ROB_Prev,0,sizeof(ROB_Prev));
        memset(ROB_Next,0,sizeof(ROB_Next));
    };

    //更新上个周期执行结束的Ex和SLB信息和上个周期传来的issue
    void RobPrint(){
        std::cout<<"Commit:ROB"<<'\n';
        std::cout<<RobHead_Prev<<' '<<RobCount_Prev<<'\n';
        if(RobHead_Prev<RobCount_Prev) {
            for (int i = RobHead_Prev; i < RobCount_Prev; i++) {
                std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';
            }
        }
        else {
            for(int i=RobHead_Prev;i<=RobStorage+1;i++){
                std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';

            }
            for(int i=1;i<RobCount_Prev;i++){
                std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';

            }
        }
        std::cout<<"---------------"<<'\n';
    }

    //接收Ex和SLB发来的信息
    //会更新：某个ROB的ROBid以及其下的状态
    //发出信息并把旧版本记下

    //接受issue
    //并分配新的ROBid
    //update更新上面Ex和SLB传来的信息
    //仅需判断分支是否正确，不正确就发射清空信号
    //其它无需发射信号
    //接收RobLaunch的消息
    //尝试提交队头
    //提交成功后，发射提交信息到RS，SLB中去
    //把修改寄存器信息发到RegFile中去
    void Commit(){
   //     Print();
   ReOrderUnit RobTmp=ROB_Prev[RobHead_Prev];
   //Print();
        if (RobTmp.isFinish) {
            if(RobTmp.OpType==Type::E){
                throw End();
            }
            else if((RobTmp.OpType==Type::B&&!RobTmp.Value1)||RobTmp.OpType==Type::Jr){
                //并且要清空
                PcSign=RobTmp.Value2;
                IsClear=NewIsClear=True;
            /*    std::cout<<"J"<<'\n';
                std::cout<<RobTmp.pc<<'\n';*/
                //下一个周期的Update清空所有;
            }
            else if(RobTmp.OpType==Type::B&&RobTmp.Value1||RobTmp.OpType==Type::J){
                PcSign=RobTmp.Value2;
            }
            else {
                int tmp=reg.ReadReg(32)+4;
                PcSign=tmp;
            }
            if(RobTmp.OpType==Type::S) {
                CommitSlbSign.first=1;
                CommitSlbSign.second=RobHead_Prev;
            }
            else {
                CommitSlbSign.first=0;
                CommitSlbSign.second=RobHead_Prev;
            }
            //  std::cout<<RobHead_Prev<<' '<<ROB_Prev[RobHead_Prev].pc<<'\n';
          //  if(ROB_Prev[RobHead_Prev].pc==4132) reg.Print();
            CommitSign.first=RobHead_Prev;
            CommitSign.second.second=RobTmp.rd;
            CommitSign.second.first=RobTmp.Value1;
            if(StuckRob==RobHead_Prev&&IsStuck&&RobTmp.OpType!=Type::S) {
                NewIsStuck=false;
            }
            ROB_Next[RobHead_Prev].isFinish=ROB_Prev[RobHead_Prev].isFinish=false;
            ROBsize_Next=ROBsize_Prev=ROBsize_Prev-1;
            if(RobHead_Prev==RobMod) RobHead_Next=RobHead_Prev=1;
            else RobHead_Next=++RobHead_Prev;
        }
        else{
            CommitSign.first=0;
            CommitSign.second.second=0;
            CommitSign.second.first=0;
        }
        //   std::cout<<"------------"<<'\n';

        //这时才会更新count和size值
        //并由RS和Slb接收
    }





    int LoadExecute(Issue &issue,int & Value1) {
        //lb lh lw lbu lhu
        int immediate = issue.immediate;
        Sext(immediate, 20);
        int ans = 0;
        switch (issue.funct3) {
            case 0://lb
                ans = ram[Value1 + immediate];
                Sext(ans, 24);
                break;
            case 1: {//lh
                int tmp = 0;
                int Byte;
                int addr = Value1 + immediate;
                for (int i = 0; i < 2; i++) {
                    Byte = ram[addr + i];
                    tmp += (Byte << (8 * i));
                }
                ans = tmp;
                Sext(ans, 16);
                break;
            }
            case 2: {//lw
                int tmp = 0;
                int Byte;
                int addr = Value1 + immediate;
              /*  std::cout << "Load" << "\n";
                std::cout << addr << '\n';
                std::cout<<issue.pc<<'\n';
                std::cout<<issue.rd<<'\n';*/
                for (int i = 0; i < 4; i++) {
                    Byte = ram[addr + i];
                    tmp += (Byte << (8 * i));
                }
                ans = tmp;
            //    std::cout << ans << '\n';
                break;
            }
            case 4: {//lbu
                unsigned int tmp = (ram[Value1 + immediate]);
                Sext(tmp, 24);
                ans = tmp;
                break;
            }
            case 5://lhu
                unsigned int tmp = 0;
                int Byte;
                int addr = Value1 + immediate;
                for (int i = 0; i < 2; i++) {
                    Byte = ram[addr + i];
                    tmp += (Byte << (8 * i));
                }
                Sext(tmp,16);
                ans = tmp;
                break;
        }
        return ans;
    }

    void StoreExecute(Issue &issue,int Value1,int Value2) {
        int immediate = issue.immediate;
        Sext(immediate, 20);
        switch (issue.funct3) {
            case 0: {
                unsigned int tmp = Value2;
                unsigned char Byte;
                SufBin(Byte, tmp, 8);
                ram[Value1 + immediate] = Byte;
                break;//sb
            }
            case 1: {
                unsigned char Byte;
                unsigned int tmp = Value2;
                int addr = Value1 + immediate;
                for (int i = 0; i < 2; i++) {
                    SufBin(Byte, tmp, 8);
                    ram[addr + i] = Byte;
                }
                break;
            }//sh
            case 2: {
                unsigned char Byte;
                unsigned int tmp = Value2;
                int addr = Value1 + immediate;
              /*  std::cout << "Store" << '\n';
                std::cout<<Value1<<"\n";
                std::cout<<immediate<<'\n';
                std::cout << addr << "\n";
                std::cout << tmp << '\n';
                std::cout<<issue.pc<<'\n';*/
                for (int i = 0; i < 4; i++) {
                    SufBin(Byte, tmp, 8);
                    ram[addr + i] = Byte;
                }
                break;
            }//sw
        }
    }


    void StoreLoadBufferInitial() {
        BAsize_Next = BAsize_Prev = 0;
        SlbCount_Prev = SlbCount_Next = SlbHead_Prev = SlbHead_Next = 1;
        memset(SLB_Prev,0,sizeof(SLB_Prev));
        memset(SLB_Next,0,sizeof(SLB_Next));
    }

    //接受issue信息
    void IssueReceive() {
        if ((!IsStuck)) {

            if(IssueSign.opcode==19&&IssueSign.immediate==255&&IssueSign.rd==10&&IssueSign.rs1==0){
                IssueSign.OpType=Type::E;
            }
            NewRob(IssueSign.OpType, IssueSign.rd,IssueSign.pc);
            //  std::cout<<Index<<'\n';
            if(RobStoreStuck_Prev[RobCount_Next]){
                NewIsStuck=true;
                StuckRob=RobCount_Next;
            }
            else if(ROBsize_Next==RobStorage) {
                NewIsStuck=true;
                StuckRob=RobHead_Prev;
            }
            IssueSign.ROBid=Index;

            if (IssueSign.OpType == Type::S || IssueSign.OpType == Type::L) {
                NewAddr(IssueSign);
                if (BAsize_Next == SlbStorage) {
                    NewIsStuck=true, StuckRob=SLB_Prev[SlbHead_Prev].ExIssue.ROBid;
                }
            }
            else {
                NewUnit(IssueSign);
                //     Print();
                if(RsSize_Next==RsStorage) {
               //     std::cout<<"RsStuck"<<'\n';
                    //    Print();
                    NewIsStuck=true, StuckRob=IssueSign.ROBid;
                }
                //这个时候会更新占用寄存器的ROB
            }

        }
    }
    //如果是读写 则缓存新的读写
    //同时接受ROB上周期Commit的信息
    //把在Q的值更新缓存
    void FunReceive(){
        if(!IsEmpty) {
                std::pair<std::pair<int,int>, std::pair<int,int>> tExSign = ExSign;//Ex信息包括了结果与哪个ROBid
                ROB_Next[tExSign.first.first].Value1=tExSign.second.first;
                ROB_Next[tExSign.first.first].Value2=tExSign.second.second;
                ROB_Next[tExSign.first.first].isFinish=true;
                std::pair<int, std::pair<int,int>> tSlbSign = SlbSign;//Slb信息包括ROBid,Load的值
                if(ROB_Next[tSlbSign.first].OpType==Type::L) {
                    ROB_Next[tSlbSign.first].Value1 = tSlbSign.second.first;
                    ROB_Next[tSlbSign.first].isFinish = true;
                }


            int RobId = tExSign.first.first;
            if (RobId && tExSign.first.second && !IsClear) {
                for (int Cur = RS_Prev[RsHead].Nxt; Cur != RsRear; Cur = RS_Prev[Cur].Nxt) {
                    if (RS_Prev[Cur].Pi.Rob == RobId) {
                        RS_Next[Cur].Pi.Rob = 0;
                        RS_Next[Cur].Pi.Data = tExSign.second.first;
                    }
                    if (RS_Prev[Cur].Pj.Rob == RobId) {
                        RS_Next[Cur].Pj.Rob = 0;
                        RS_Next[Cur].Pj.Data = tExSign.second.first;
                    }
                }
                for (int i = 0; i <= SlbMod; i++) {
                    if (RobId == SLB_Prev[i].Data1.Rob && SLB_Prev[i].isBusy) {
                        SLB_Next[i].Data1.Rob = 0;
                        SLB_Next[i].Data1.Data = tExSign.second.first;
                    }
                    if (RobId == SLB_Prev[i].Data2.Rob && SLB_Prev[i].isBusy) {
                        SLB_Next[i].Data2.Rob = 0;
                        SLB_Next[i].Data2.Data = tExSign.second.first;
                    }
                }

            }
            RobId = tSlbSign.first;
            if (RobId && tSlbSign.second.second && !IsClear) {
                for (int Cur = RS_Prev[RsHead].Nxt; Cur != RsRear; Cur = RS_Prev[Cur].Nxt) {
                    if (RS_Prev[Cur].Pi.Rob == RobId) {
                        RS_Next[Cur].Pi.Rob = 0;
                        RS_Next[Cur].Pi.Data = tSlbSign.second.first;
                    }
                    if (RS_Prev[Cur].Pj.Rob == RobId) {
                        RS_Next[Cur].Pj.Rob = 0;
                        RS_Next[Cur].Pj.Data = tSlbSign.second.first;
                    }
                }
                for (int i = 0; i <= SlbMod; i++) {
                    if (RobId == SLB_Prev[i].Data1.Rob && SLB_Prev[i].isBusy) {
                        SLB_Next[i].Data1.Rob = 0;
                        SLB_Next[i].Data1.Data = tSlbSign.second.first;
                    }
                    if (RobId == SLB_Prev[i].Data2.Rob && SLB_Prev[i].isBusy) {
                        SLB_Next[i].Data2.Rob = 0;
                        SLB_Next[i].Data2.Data = tSlbSign.second.first;
                    }
                }
            }
        }
    }
    void SlbCommitReceive(){
        if(!IsEmpty) {
            std::pair<int,int> tCommitSlbSign = CommitSlbSign;
            if(tCommitSlbSign.first){
                for (int i = 0; i <= SlbStorage + 1; i++) {
                    if (tCommitSlbSign.second == SLB_Prev[i].ExIssue.ROBid && SLB_Prev[i].isBusy&&SLB_Prev[i].ExIssue.OpType == Type::S){
                        SLB_Next[i].isCommit = true;
                        break;
                    }
                }
            }
        }
        //SLB包括了计算地址 计算地址等时钟走完再计算
        //收包括ROB的信息，包括上个周期issue的信息，并把相应的谁释放了发到下个周期的ROB里
    }
    //根据新信息，判断SLB头能否操作
    //如果是读寄存器 等待该语句Commit后操作
    //如果是写寄存器 直接开始操作
    //并把结果和寄存器丢到ROB里去
    //把新信息缓存
    //在Update更新始终
    //若头走完时钟周期,则交给ROB相应的值
    void Launch() {
        if(!IsEmpty) {
            bool flag = false;
            int Value1=0, Value2=0;
            Issue ExIssue;
            ExIssue.SetZero();
            for (int Cur = RS_Next[RsHead].Nxt; Cur != RsRear; Cur = RS_Next[Cur].Nxt) {
                if ((!RS_Next[Cur].Pi.Rob) && (!RS_Next[Cur].Pj.Rob)) {
                    Value1 = RS_Next[Cur].Pi.Data;
                    Value2 = RS_Next[Cur].Pj.Data;
                    int Prev=RS_Next[Cur].Prv;
                    int Next=RS_Next[Cur].Nxt;
                    Pool[--RsTop]=Cur;
                    RS_Next[Prev].Nxt=Next;
                    RS_Next[Next].Prv=Prev;
                    ExIssue = RS_Next[Cur].ExIssue;
                    RsSize_Next=RsSize_Prev-1;

                    break;
                }
            }
            //  std::cout<<ExIssue.ROBid<<' '<<ExIssue.pc<<'\n';
            RsSign.first=ExIssue;
            RsSign.second.first=Value1;
            RsSign.second.second=Value2;
            //还有没有可执行的话，发射什么?
            //阻塞了
            //发射准备好的RS到Ex里面
        }
        else {
            Issue Tmp;
            Tmp.SetZero();
        }

        if(SLB_Next[SlbHead_Next].isCommit&&SLB_Next[SlbHead_Next].isBusy&&!SLB_Next[SlbHead_Next].Data1.Rob&&!SLB_Next[SlbHead_Next].Data2.Rob) {
            SLB_Next[SlbHead_Next].clock--;
            BusyAddress BaTmp = SLB_Next[SlbHead_Next];
            switch (BaTmp.ExIssue.OpType) {
                case Type::S: {
                    if (!BaTmp.clock) {
                       /* std::cout<<"S"<<'\n';
                        std::cout<<BaTmp.ExIssue.ROBid<<'\n';
                        std::cout<<StuckRob<<'\n';
                        std::cout<<isStuck()<<'\n';*/
                        //计时结束 写内存
                        StoreExecute(BaTmp.ExIssue,BaTmp.Data1.Data,BaTmp.Data2.Data);
                        //并将完成信息丢给ROB的Receive;
                        RobStoreStuck_Next[BaTmp.ExIssue.ROBid]=false;
                    //    std::cout<<BaTmp.ExIssue.pc<<'\n';
                        SlbSign.first=BaTmp.ExIssue.ROBid;
                        SlbSign.second.first=0;
                        SlbSign.second.second=0;
                        if(IsStuck&&StuckRob==BaTmp.ExIssue.ROBid){
                          //  std::cout<<"!"<<'\n';
                            NewIsStuck=False;
                        }
                        SLB_Next[SlbHead_Next].isBusy=false;
                        if(SlbHead_Prev==SlbMod) SlbHead_Next=1;
                        else ++SlbHead_Next;
                        //SlbHead_Next=SlbHead_Prev%(SlbStorage+1)+1;
                        BAsize_Next=BAsize_Prev-1;
                    }//阻塞
                    else {
                        SlbSign.first=0;
                        SlbSign.second.first=0;
                        SlbSign.second.second=0;
                    }
                    break;
                }
                case Type::L: {
                    if (!BaTmp.clock) {
                   int ans= LoadExecute(BaTmp.ExIssue,BaTmp.Data1.Data);//把值从内存里读出来写到BaTmp里
                        //哪个ROB完成了
                        SlbSign.first=BaTmp.ExIssue.ROBid;
                        SlbSign.second.first=ans;
                        SlbSign.second.second=BaTmp.ExIssue.rd;
                     //   if(isStuck()&&StuckRob==BaTmp.ExIssue.ROBid) SetStuck(false);
                        SLB_Next[SlbHead_Next].isBusy=false;
                        if(SlbHead_Prev==SlbMod) SlbHead_Next=1;
                        else ++SlbHead_Next;
                        //SlbHead_Next=SlbHead_Prev%(SlbStorage+1)+1;
                        BAsize_Next=BAsize_Prev-1;
                        //给的是ROB与计算结果
                        //要写某个寄存器了并且将信息丢给ROB的Receive
                    } else {
                        SlbSign.first=0;
                        SlbSign.second.first=0;
                        SlbSign.second.second=0;}//阻塞并等待
                    break;
                }
            }
        }
        else{SlbSign.first=0;
            SlbSign.second.first=0;
            SlbSign.second.second=0;}
            //阻塞
            //把谁释放了发给下个周期的ROB
        //会计算时钟
    }
    void Renew(){

        RobCount_Prev = RobCount_Next;
        ROBsize_Prev = ROBsize_Next;
        RobHead_Prev = RobHead_Next;
        if(RobHead_Prev<RobCount_Prev){
            for(int i=RobHead_Prev;i<RobCount_Prev;i++)
                ROB_Prev[i]=ROB_Next[i];
        }
        else{
            for(int i=RobHead_Prev;i<=RobStorage+1;i++)
                ROB_Prev[i]=ROB_Next[i];
            for(int i=1;i<RobCount_Prev;i++)
                ROB_Prev[i]=ROB_Next[i];
        }
        memcpy(RobStoreStuck_Prev,RobStoreStuck_Next,sizeof(RobStoreStuck_Prev));


        memcpy(RS_Prev,RS_Next,sizeof(RS_Prev));
        RsSize_Prev = RsSize_Next;


   //     if(isStuck()) {
        SlbHead_Prev = SlbHead_Next;
            SlbCount_Prev = SlbCount_Next;
            BAsize_Prev = BAsize_Next;
            memcpy(SLB_Prev,SLB_Next,sizeof(SLB_Prev));
         //   for (int i = 0; i <= SlbStorage+1; i++) SLB_Prev[i] = SLB_Next[i];
    //    }
        //如果阻塞把头commit改回false
        //且时钟不会更新

    }
    void Clear(){
        RobCount_Next=RobHead_Next;
        ROBsize_Next=0;
        memset(ROB_Next,0,sizeof(ROB_Next));

        RsSize_Next=0;
        memset(RS_Next,0,sizeof(RS_Next));
        RsTop=0;
        for(int i=0;i<10;i++) Pool[i]=i+1;
        RS_Next[RsHead].Nxt=RsRear;
        RS_Next[RsRear].Prv=RsHead;


        int tmp=0;
        for(int i=0;i<=SlbStorage+1;i++){
            if(SLB_Next[i].ExIssue.OpType==Type::S&&SLB_Next[i].isBusy&&SLB_Next[i].isCommit){
                tmp++;
                RobStoreStuck_Next[SLB_Next[i].ExIssue.ROBid]=true;
            }
            else SLB_Next[i].isCommit=SLB_Next[i].isBusy=false;
        }
        int Cal=SlbHead_Prev+tmp-1;

        SlbCount_Next=(SlbHead_Prev+tmp-1)%(SlbStorage+1)+1;
        BAsize_Next=tmp;
    }

//DecodingTool decode;


/*    int Head_Prev;
    int Head_Next;*/
    //记在等待提交的ROBid对应的
    //记与提交的ROBid对应的

    void ReserveStationInitial(){
        RsSize_Prev=RsSize_Next=0;
        for(int i=0;i<10;i++) Pool[i]=i+1;
        RsTop=0;
        memset(RS_Prev,0,sizeof(RS_Prev));
        memset(RS_Next,0,sizeof(RS_Next));
        RS_Prev[RsHead].Nxt=RS_Next[RsHead].Nxt=RsRear;
        RS_Prev[RsRear].Prv=RS_Next[RsRear].Prv=RsHead;
        RS_Prev[RsHead].Prv=RS_Next[RsHead].Prv=RS_Next[RsRear].Nxt=RS_Prev[RsRear].Nxt=-1;
    }

    //接受Issue传来的信息，如果是函数类
    //则缓存新信息，等待更新

    void RsPrint(){
        std::cout<<"Rs"<<'\n';
        std::cout<<RS_Prev[RsHead].Nxt<<'\n';
        for (int Cur = RS_Prev[RsHead].Nxt; Cur != RsRear; Cur = RS_Prev[Cur].Nxt){
            std::cout<<Cur<<' '<<RS_Prev[Cur].Pi.Rob<<' '<<RS_Prev[Cur].Pj.Rob<<' '<<RS_Prev[Cur].ExIssue.pc<<' ';
            std::cout<<RS_Prev[Cur].Prv<<" "<<RS_Prev[Cur].Nxt<<'\n';
        }
    }

    //发射准备好的Rs到Ex去，
    //可以给出准备好的值
    //接受哪一个ROB完成了 可能不一定是ROB头
    //然后根据完成的值 更新Rs里的值
    //同时缓存Commit的影响

    //更新Issue来的
    //更新等待的
    //更新对应的

class ExecutePart{
private:
    ExecuteTool execute;
public:
    ExecutePart()=default;
    ~ExecutePart()=default;
    void Execute(){
            std::pair<Issue, std::pair<int, int>> tRsSign = RsSign;
            if(tRsSign.first.ROBid) {
                std::pair<int,int> ans = execute.Exec(tRsSign.first, tRsSign.first.pc, tRsSign.second.first, tRsSign.second.second);
                ExSign.first.first=tRsSign.first.ROBid;
                ExSign.first.second=tRsSign.first.rd;
                ExSign.second.first=ans.first;
                ExSign.second.second=ans.second;
               /* std::cout<<"eX"<<'\n';
                std::cout<<tRsSign.first.ROBid<<' '<<tRsSign.first.pc<<"\n";*/
            }
            else{
                ExSign.first.first=0;
                ExSign.first.second=0;
                ExSign.second.first=0;
                ExSign.second.second=0;
            }
    }
};
class PipeLing{
private:
    BinOrderCache Boc;
    ExecutePart Ex;
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
public:
    PipeLing()=default;
     int Run(){
        //
        ControlBusInitial();
        ReOrderInitial();
        ReserveStationInitial();
        StoreLoadBufferInitial();
        try {
            while (true) {
                FunReceive();
                SlbCommitReceive();
                reg.CommitReceive();
                if(IsClear){
                    Clear();
                    reg.Clear();
                    Boc.Clear();
                    IsClear=NewIsClear=False;
                    NewIsStuck=false;
               //     std::cout<<"Clear"<<'\n';
                }

                Launch();

                Boc.Fetch();

                Renew();
                Boc.Renew();
                IsEmpty=NewIsEmpty;
                IsStuck=NewIsStuck;
                IsClear=NewIsClear;

              //  reg.Print();
                Ex.Execute();
                Boc.Decode();

                IssueReceive();
                reg.IssueReceive();

                Commit();
                reg.PcReceive();
                IsStuck=NewIsStuck;
              //  std::cout<<i<<'\n';
            //    std::cout<<i<<"\n";
            }
        }catch(End){/**/
            /*std::cout<<i<<"\n";
            std::cout<<'?'<<'\n';*/
            return (unsigned (reg.ReadReg(10)))&(255u);
        }
    }
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
      //  std::cout<<'/'<<'\n';
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
    ~PipeLing()=default;
};
#endif //RISCV_PIPELING_H
