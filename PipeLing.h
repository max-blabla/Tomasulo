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
int Zero=0;
int LazyFlag=-1;
bool False=false;
bool True=true;
class ControlBus{
    int BocSign;
    Issue IssueSign;
    std::pair<Issue,std::pair<int,int>> RsSign;
    std::pair<int,std::pair<int,int>> ExSign;
    std::pair<int,int> SlbSign;
    std::pair<int,int> RobSlbSign;
    std::pair<int,std::pair<int,int>> CommitSign;
    bool IsEmpty=true;
    bool NewIsEmpty=true;
    bool IsClear=false;
    bool IsStuck=false;
    bool NewIsStuck=false;
    bool NewIsClear=false;
    int StuckRob;
    int PcSign;
public:
    ControlBus()=default;
    void Renew(){
        //更新信号版本
        IsEmpty=NewIsEmpty;
        IsStuck=NewIsStuck;
        IsClear=NewIsClear;
    }
    void BocSet(int & BOCSign){
        BocSign=BOCSign;
    }

    void StuckLaunch(int & ROB){
        StuckRob=ROB;
    }
    int StuckROB(){

        return StuckRob;
    }
    int BocResult(){
        return BocSign;
    }
    void PcLaunch(int & Pc){
        PcSign=Pc;
    }
    int PcResult(){
        return PcSign;
    }
    //记ROBid和值
    //如果是写,rd设为0
    std::pair<int,int> SlbResult(){
        //返回都是旧版本的
        return SlbSign;
    }
    void SlbLaunch(int & RobId,int & Value){
        SlbSign.first=RobId;
        SlbSign.second=Value;
    }
    void RobSlbLaunch(int & RobId,int &rd){
        RobSlbSign.first=RobId;
        RobSlbSign.second=rd;
    }
    //ExResult 为Ex函数们执行的信息，第一维是ROBid，第二维是返回值
    std::pair<int,std::pair<int,int>> ExResult(){
        return ExSign;
    }
    void ExLaunch(int & RobId,int & ans,int & Pc){
        ExSign.first=RobId;
        ExSign.second.first=ans;
        ExSign.second.second=Pc;
    }
    void CommitLaunch(int & Value,int & rd, int & ROBid){
        CommitSign.first=ROBid;
        CommitSign.second.first=Value;
        CommitSign.second.second=rd;
    }
    //first是ROBid second.first是计算结果 second.second是寄存器位置
    std::pair<int,std::pair<int,int>> CommitResult(){
        return CommitSign;
    }
    void IssueLaunch(Issue & DataPack){
        IssueSign=DataPack;
    }
    //解码后的issue，包含分配好的ROBid
    Issue IssueResult(){
        return IssueSign;
    }
    int IssueModify(int & ROBid){
        IssueSign.ROBid=ROBid;
    }
    void RsLaunch(int & V1,int & V2,Issue & issue){
        RsSign.first=issue;
        RsSign.second.first=V1;
        RsSign.second.second=V2;
    }
    //first是解码信息 second.first是Value1 second.second是Value2
    std::pair<Issue,std::pair<int,int>> RsResult(){
        return RsSign;
    }
    bool isEmpty(){
        return IsEmpty;
    }
    bool isClear(){
        return IsClear;
    }
    void SetClear(bool & Flag){
        IsClear=NewIsClear=Flag;
    }
    void SetEmpty(bool & Flag){
        NewIsEmpty=Flag;
    }
    bool isStuck(){
        return IsStuck;
    }
    void SetStuck(bool & Flag){
        NewIsStuck=Flag;
    }
    ~ControlBus()=default;
};
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
    void Clear(){
        for(int i=0;i<32;i++)
            ROBid_Next[i]=0;
    }
    RegFile()=default;
    ~RegFile()=default;
    void Renew(ControlBus & CBD){
            for (int i = 0; i < 32; i++) ROBid_Prev[i] = ROBid_Next[i];
            for (int i = 0; i <= 32; i++){ reg_Prev[i] = reg_Next[i];}
          /*  for(int i=0;i<=32;i++)
            std::cout << reg_Prev[i] << ' ';
            std::cout << '\n';*/
    }
    void CommitReceive(ControlBus & CBD){
        int rd=CBD.CommitResult().second.second;
        if(ROBid_Next[rd]==CBD.CommitResult().first) ROBid_Next[rd]=0;
        reg_Next[rd]=CBD.CommitResult().second.first;
    }
    void IssueReceive(ControlBus & CBD){
        if((!CBD.isStuck())) {
            int rd = CBD.IssueResult().rd;
            ROBid_Prev[rd]=ROBid_Next[rd] = CBD.IssueResult().ROBid;
        }
        //WriteRob(IssueSign.rd,IssueSign.ROBid);
    }
    void PcReceive(ControlBus & CBD){
        reg_Prev[32]=reg_Next[32]=CBD.PcResult();
      //  if(!CBD.isEmpty()) WriteRob(32,CBD.PcResult());
    }
    int ReadRob(int & addr){
        return ROBid_Prev[addr];
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
    std::pair<int,int> Cache_Prev[Storage];
    std::pair<int,int> Cache_Next[Storage];
    int Head_Prev;
    int Head_Next;
    int BOCsize_Prev;
    int BOCsize_Next;
    int Count_Prev;
    int Count_Next;
    int FetchPc=0;
public:
    BinOrderCache(){
        Head_Prev=Head_Next=BOCsize_Prev=BOCsize_Next=Count_Prev=Count_Next=0;
    }
    void PushBack(int & BinOrder){
        Cache_Next[Count_Prev].first=BinOrder;
        Cache_Next[Count_Prev].second=FetchPc;
        BOCsize_Next=BOCsize_Prev+1;
        Count_Next=(Count_Prev+1)%Storage;
    }
    void PopFront(){
        Head_Next=Head_Prev=(Head_Prev+1)%Storage;
        BOCsize_Next=BOCsize_Prev=BOCsize_Prev-1;
    }
    void Fetch(ControlBus & CBD){
     //   std::cout<<"Fetch"<<' '<<FetchPc<<'\n';
        int BinOrder = ram.LoadOrder(FetchPc);
        //reg.WriteReg(32,pc+4)
        //;
        if(BOCsize_Prev<=Storage-2) {
            PushBack(BinOrder);
            FetchPc += 4;
        }
        else{
            std::cout<<"Can?"<<'\n';
        }
        if(CBD.isEmpty()) CBD.SetEmpty(False);
    }
    void Decode(ControlBus & CBD){
        int BinOrder=Cache_Prev[Head_Prev].first;
        int Pc=Cache_Prev[Head_Prev].second;
        Issue issue=decode.decoding(BinOrder);
        issue.pc=Pc;
        CBD.IssueLaunch(issue);
        if((!CBD.isStuck())) PopFront();
    }
    void Renew(ControlBus & CBD){
       // if(!CBD.isStuck()) {
            Head_Prev = Head_Next;
            BOCsize_Prev = BOCsize_Next;
            Count_Prev = Count_Next;
            for (int i = 0; i < Storage; i++) Cache_Prev[i] = Cache_Next[i];
       // }
    }
    void Clear(){
        Head_Prev=Head_Next=BOCsize_Prev=BOCsize_Next=Count_Prev=Count_Next=0;
        FetchPc=reg.ReadReg(32);
    }
    ~BinOrderCache()=default;
};
class StoreLoadBuffer {
private:
    struct BusyAddress {
        bool isCommit = false;
        RegFile::RegPair Data1;
        RegFile::RegPair Data2;
        bool isBusy = false;
        Issue ExIssue;
        int clock;
    };
    static const int Storage = 3;
    BusyAddress SLB_Prev[Storage + 2];
    BusyAddress SLB_Next[Storage + 2];
    int Head_Prev;
    int Head_Next;
    int BAsize_Prev;
    int BAsize_Next;
    int Count_Prev;
    int Count_Next;

    bool NewAddr(Issue &issue) {

        /*     std::cout<<"Count_Next"<<' '<<Count_Next<<'\n';
             std::cout<<issue.pc<<'\n';*/
        SLB_Prev[Count_Prev].ExIssue=SLB_Next[Count_Prev].ExIssue = issue;
        SLB_Prev[Count_Prev].isBusy=SLB_Next[Count_Prev].isBusy = true;
        RegFile::RegPair Q1=reg.ReadData(issue.rs1);
        RegFile::RegPair Q2=reg.ReadData(issue.rs2);
        SLB_Prev[Count_Prev].Data1=SLB_Next[Count_Prev].Data1=Q1;
        SLB_Prev[Count_Prev].Data2=SLB_Next[Count_Prev].Data2=Q2;
        if (issue.OpType == Type::L)
            SLB_Prev[Count_Prev].isCommit = SLB_Next[Count_Prev].isCommit = true;
            //if(!P.Rob) SLB_Prev[Count_Prev].Data1=SLB_Next[Count_Next].Data1=P.Data;
        else
            SLB_Prev[Count_Prev].isCommit = SLB_Next[Count_Prev].isCommit = false;
        SLB_Prev[Count_Prev].clock=SLB_Next[Count_Prev].clock = 3;
        Count_Prev=Count_Next = (Count_Prev) % (Storage+1) + 1;
        BAsize_Prev=BAsize_Next = BAsize_Prev + 1;
        return true;
    }

    static int LoadExecute(Issue &issue,int Value1) {
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

    static void StoreExecute(Issue &issue,int Value1,int Value2) {
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

public:
    StoreLoadBuffer() {
        BAsize_Prev = BAsize_Prev = 0;
        Count_Prev = Count_Next = Head_Prev = Head_Next = 1;
    }

    //接受issue信息
    void IssueReceive(ControlBus &CBD,int & i) {
        if ((!CBD.isStuck())) {
            Issue issue = CBD.IssueResult();
            if (issue.OpType == Type::S || issue.OpType == Type::L) {
                NewAddr(issue);
                if (BAsize_Next == Storage) {
                        CBD.SetStuck(True), CBD.StuckLaunch(issue.ROBid);

                }
                //记寄存器被谁占了
            }
            else {}
        }
    }
    //如果是读写 则缓存新的读写
    //同时接受ROB上周期Commit的信息
    //把在Q的值更新缓存
    void CommitReceive(ControlBus & CBD){
        if(!CBD.isEmpty()) {
            std::pair<int, std::pair<int, int>> CommitSign = CBD.CommitResult();
            for(int i=0;i<=Storage+1;i++) {
                if (CommitSign.first == SLB_Prev[i].ExIssue.ROBid && SLB_Prev[i].isBusy) {//HeadCommit
                    if(SLB_Prev[i].ExIssue.OpType==Type::S) {
                        SLB_Next[i].isCommit = true;
                    }
                }
                if(SLB_Prev[i].Data1.Rob&&CommitSign.first==SLB_Prev[i].Data1.Rob&&SLB_Prev[i].isBusy){
                        SLB_Next[i].Data1.Rob = 0;
                        SLB_Next[i].Data1.Data = CommitSign.second.first;
                }
                if(SLB_Prev[i].Data2.Rob&&CommitSign.first==SLB_Prev[i].Data2.Rob&&SLB_Prev[i].isBusy){
                        SLB_Next[i].Data2.Rob = 0;
                        SLB_Next[i].Data2.Data = CommitSign.second.first;
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
    void Launch(ControlBus & CBD) {
      //  std::cout<<"Slb:Launch"<<' '<<Head_Next<<'\n';
     /* std::cout<<"-----------"<<"\n";
      std::cout<<"Slb:Launch"<<'\n';
      std::cout<<Head_Next<<' '<<Count_Next<<'\n';
      for(int i=Head_Next;i<Count_Next;i++)
          std::cout<<SLB_Next[i].ExIssue.pc<<' '<<SLB_Next[i].isCommit<<'\n';
      std::cout<<"-----------"<<"\n";*/
   /*  std::cout<<"SLB"<<'\n';
     //!!!ROB归还
     std::cout<<Head_Next<<" "<<Count_Next<<'\n';
     if(Head_Next<=Count_Next) {
         for (int i = Head_Next; i < Count_Next; i++)
             std::cout << SLB_Next[i].ExIssue.pc << ' ' <<SLB_Next[i].isBusy<<' '<< SLB_Next[i].isCommit<<' ' <<SLB_Next[i].clock<<' '<<SLB_Next[i].ExIssue.ROBid << '\n';
     }
     else{
         for(int i=Head_Next;i<=Storage+1;i++)
             std::cout << SLB_Next[i].ExIssue.pc << ' ' << SLB_Next[i].isBusy<<' '<<SLB_Next[i].isCommit <<' '<<SLB_Next[i].clock <<' '<<SLB_Next[i].ExIssue.ROBid<< '\n';
         for(int i=1;i<Count_Next;i++)
             std::cout << SLB_Next[i].ExIssue.pc << ' ' << SLB_Next[i].isBusy<<' '<<SLB_Next[i].isCommit <<' '<<SLB_Next[i].clock <<' '<<SLB_Next[i].ExIssue.ROBid <<'\n';
     }*/
        if(SLB_Next[Head_Next].isCommit&&SLB_Next[Head_Next].isBusy&&!SLB_Next[Head_Next].Data1.Rob&&!SLB_Next[Head_Next].Data2.Rob) {
            SLB_Next[Head_Next].clock--;
            BusyAddress BaTmp = SLB_Next[Head_Next];
            switch (BaTmp.ExIssue.OpType) {
                case Type::S: {
                    if (!BaTmp.clock) {
                        //计时结束 写内存
                        StoreExecute(BaTmp.ExIssue,BaTmp.Data1.Data,BaTmp.Data2.Data);
                        //并将完成信息丢给ROB的Receive;
                        CBD.SlbLaunch(BaTmp.ExIssue.ROBid, Zero);
                        if(CBD.isStuck()&&CBD.StuckROB()==BaTmp.ExIssue.ROBid){
                            CBD.SetStuck(False);
                        }
                        SLB_Next[Head_Next].isBusy=false;
                        Head_Next=Head_Prev%(Storage+1)+1;
                        BAsize_Next=BAsize_Prev-1;
                    }//阻塞
                    else CBD.SlbLaunch(Zero,Zero);
                    break;
                }
                case Type::L: {
                    if (!BaTmp.clock) {
                   int ans= LoadExecute(BaTmp.ExIssue,BaTmp.Data1.Data);//把值从内存里读出来写到BaTmp里
                        //哪个ROB完成了
                        CBD.SlbLaunch(BaTmp.ExIssue.ROBid, ans);
                     //   if(CBD.isStuck()&&CBD.StuckROB()==BaTmp.ExIssue.ROBid) CBD.SetStuck(false);
                        SLB_Next[Head_Next].isBusy=false;
                        Head_Next=Head_Prev%(Storage+1)+1;
                        BAsize_Next=BAsize_Prev-1;
                        //给的是ROB与计算结果
                        //要写某个寄存器了并且将信息丢给ROB的Receive
                    } else {CBD.SlbLaunch(Zero,Zero);}//阻塞并等待
                    break;
                }
            }
        }
        else CBD.SlbLaunch(Zero,Zero);
            //阻塞
            //把谁释放了发给下个周期的ROB
        //会计算时钟
    }
    void Renew(ControlBus & CBD){
   //     if(CBD.isStuck()) {
            Head_Prev = Head_Next;
            Count_Prev = Count_Next;
            BAsize_Prev = BAsize_Next;
            for (int i = 0; i <= Storage+1; i++) SLB_Prev[i] = SLB_Next[i];
    //    }
        //如果阻塞把头commit改回false
        //且时钟不会更新

    }
    void Clear(){
        int tmp=0;
        for(int i=0;i<=Storage+1;i++){
            if(SLB_Next[i].ExIssue.OpType==Type::S&&SLB_Next[i].isBusy&&SLB_Next[i].isCommit){
                tmp++;
            }
            else SLB_Next[i].isCommit=SLB_Next[i].isBusy=false;
        }
        Count_Next=(Head_Prev+tmp-1)%(Storage+1)+1;
        BAsize_Next=tmp;
    }
};
//DecodingTool decode;
class ReOrderBuffer {
private:
    static const int Storage=24;
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
    ReOrderUnit ROB_Prev[Storage+2];
    ReOrderUnit ROB_Next[Storage+2];
    int Head_Prev;
    int Head_Next;
    int Count_Prev;
    int Count_Next;
    //缓存新信息
    int Index;
    void NewRob(Type & type,int & rd,int & pc){

       Index=Count_Prev;
        ROB_Prev[Count_Prev].OpType=ROB_Next[Count_Prev].OpType=type;
        ROB_Prev[Count_Prev].rd=ROB_Next[Count_Prev].rd=rd;
        ROB_Prev[Count_Prev].pc=ROB_Next[Count_Prev].pc=pc;
        ROB_Prev[Count_Prev].isFinish=ROB_Next[Count_Prev].isFinish=false;
        if(type==Type::S)  ROB_Prev[Count_Next].isFinish=ROB_Next[Count_Prev].isFinish=true;
        Count_Prev=Count_Next=Count_Prev%(Storage+1)+1;
        ROBsize_Prev=ROBsize_Next=ROBsize_Prev+1;
    }
   //新issue对应Adder
   //接受函数执行结果
   //接受读写结果
    //一个周期内可能两个结果都接受
public:
    ReOrderBuffer() {
        Head_Prev=Head_Next=1;
        ROBsize_Prev=ROBsize_Next=0;
        Count_Prev=Count_Next=1;
    };
    ~ReOrderBuffer()=default;
    //更新上个周期执行结束的Ex和SLB信息和上个周期传来的issue
    void Renew(ControlBus & CBD){
        //把接受的Ex和SLB信息更新
     //   if(!CBD.isStuck()) {
            Count_Prev = Count_Next;
            ROBsize_Prev = ROBsize_Next;
            Head_Prev = Head_Next;
            for (int i = 1; i <= Storage+1; i++) {
                ROB_Prev[i] = ROB_Next[i];
                //把接受的issue更新
     //       }
        }
    }
    //接收Ex和SLB发来的信息
    //会更新：某个ROB的ROBid以及其下的状态
    //发出信息并把旧版本记下
    void FunReceive(ControlBus & CBD){
        if(!CBD.isEmpty()) {
            std::pair<int, std::pair<int,int>> ExSign = CBD.ExResult();//Ex信息包括了结果与哪个ROBid
            if(ROB_Next[ExSign.first].OpType==Type::B||ROB_Next[ExSign.first].OpType==Type::J){
                ROB_Next[ExSign.first].Value1=ExSign.second.first;
                ROB_Next[ExSign.first].Value2=ExSign.second.second;
            }
            else {
                ROB_Next[ExSign.first].Value1 = ExSign.second.first;
            }
            ROB_Next[ExSign.first].isFinish=true;
            //要写到哪个里面
            std::pair<int, int> SlbSign = CBD.SlbResult();//Slb信息包括ROBid,Load的值
            ROB_Next[SlbSign.first].Value1=SlbSign.second;
            ROB_Next[SlbSign.first].isFinish=true;
          //  std::cout<<"SlbROBID:"<<' '<<SlbSign.first<<'\n';
        }
    }
    //接受issue
    //并分配新的ROBid
    void IssueReceive(ControlBus & CBD){
        if((!CBD.isStuck())) {
            Issue IssueSign = CBD.IssueResult();
            if(IssueSign.opcode==19&&IssueSign.immediate==255&&IssueSign.rd==10&&IssueSign.rs1==0){
                IssueSign.OpType=Type::E;
            }
            NewRob(IssueSign.OpType, IssueSign.rd,IssueSign.pc);
          if(ROBsize_Next==Storage) {
              CBD.SetStuck(True);
              CBD.StuckLaunch(Index);
          }
            CBD.IssueModify(Index);
        }
    }
    //update更新上面Ex和SLB传来的信息
    //仅需判断分支是否正确，不正确就发射清空信号
    //其它无需发射信号
    void Launch(ControlBus & CBD){
              //分支错误 只在Commit时判断
    }
    //接收RobLaunch的消息
    //尝试提交队头
    //提交成功后，发射提交信息到RS，SLB中去
    //把修改寄存器信息发到RegFile中去
    void Commit(ControlBus & CBD){
   /*  std::cout<<"---------------"<<'\n';
     std::cout<<"Commit:ROB"<<'\n';
     std::cout<<Head_Prev<<' '<<Count_Prev<<'\n';
     if(Head_Prev<=Count_Prev) {
         for (int i = Head_Prev; i < Count_Prev; i++) {
             std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';
         }
     }
     else {
         for(int i=Head_Prev;i<=Storage+1;i++){
             std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';

         }
         for(int i=1;i<Count_Prev;i++){
             std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';

         }
     }
        std::cout<<"---------------"<<'\n';*/
 /*       std::cout<<"---------------"<<'\n';
        std::cout<<"Commit:ROB"<<'\n';
        std::cout<<Head_Prev<<' '<<Count_Prev<<'\n';
        if(Head_Prev<=Count_Prev) {
            for (int i = Head_Prev; i < Count_Prev; i++) {
                std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';
            }
        }
        else {
            for(int i=Head_Prev;i<=Storage+1;i++){
                std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';

            }
            for(int i=1;i<Count_Prev;i++){
                std::cout << ROB_Prev[i].pc << ' ' << ROB_Prev[i].isFinish<<' '<<ROB_Prev[i].rd << '\n';

            }
        }*/
        if (ROB_Prev[Head_Prev].isFinish) {
              if(ROB_Prev[Head_Prev].OpType==Type::E){
                  throw End();
                  }
              else if((ROB_Prev[Head_Prev].OpType==Type::B&&ROB_Prev[Head_Prev].Value1)||ROB_Prev[Head_Prev].OpType==Type::J){
                  //并且要清空

                  CBD.PcLaunch(ROB_Prev[Head_Prev].Value2);
                  CBD.SetClear(True);
                  //下一个周期的Update清空所有;
              }
              else {
                  int tmp=reg.ReadReg(32)+4;
                  CBD.PcLaunch(tmp);
              }
              CBD.CommitLaunch(ROB_Prev[Head_Prev].Value1, ROB_Prev[Head_Prev].rd, Head_Prev);
              if(CBD.StuckROB()==Head_Prev&&CBD.isStuck()&&ROB_Prev[Head_Prev].OpType!=Type::S) {

                  CBD.SetStuck(False);
              }
              ROB_Next[Head_Next].isFinish=ROB_Prev[Head_Prev].isFinish=false;
             ROBsize_Next=ROBsize_Prev=ROBsize_Prev-1;
              Head_Next=Head_Prev=Head_Prev%(Storage+1)+1;
          }
          else{
              CBD.CommitLaunch(Zero,Zero,Zero);
          }
     //   std::cout<<"------------"<<'\n';

        //这时才会更新count和size值
        //并由RS和Slb接收
    }
    void Clear(){
        Count_Next=Head_Next;
        ROBsize_Next=0;
        memset(ROB_Next,0,sizeof(ROB_Next));
        //for(int i=1;i<=Storage+1;i++) ROB_Next[i].isFinish=false;
    }
};
class ReserveStation{
private:
    struct ReserveStationUnit{
        bool isBusy=0;
        RegFile::RegPair Pi;
        RegFile::RegPair Pj;
        Issue ExIssue;
    };
    static const int Storage=10;
    ReserveStationUnit RS_Prev[Storage];
    ReserveStationUnit RS_Next[Storage];
    int RsSize_Prev;
    int RsSize_Next;
    //记在等待提交的ROBid对应的
    //记与提交的ROBid对应的
    bool NewUnit(Issue & NewIssue){
        bool flag=false;
        for(int i=0;i<Storage;i++) {
            if (!RS_Prev[i].isBusy) {
                flag=true;
                RS_Prev[i].isBusy=RS_Next[i].isBusy=true;
                RS_Prev[i].ExIssue=RS_Next[i].ExIssue=NewIssue;
                RS_Prev[i].Pi=RS_Next[i].Pi=reg.ReadData(NewIssue.rs1);
                RS_Prev[i].Pj=RS_Next[i].Pj=reg.ReadData(NewIssue.rs2);
                RsSize_Prev=RsSize_Next=RsSize_Prev+1;
                break;
            }
        }
        return flag;
    }
public:
    ReserveStation(){
        RsSize_Prev=RsSize_Next=0;
    }
    ~ReserveStation()=default;
    //接受Issue传来的信息，如果是函数类
    //则缓存新信息，等待更新
    void IssueReceive(ControlBus & CBD){
        if((!CBD.isStuck())) {
            Issue IssueSign = CBD.IssueResult();
            if (IssueSign.OpType == Type::S || IssueSign.OpType == Type::L) {}
            else {
                NewUnit(IssueSign);
                if(RsSize_Next==Storage) {
                    CBD.SetStuck(True), CBD.StuckLaunch(IssueSign.ROBid);
                }
                //这个时候会更新占用寄存器的ROB
            }
        }
        //计算并得到旧版本
    }
    //发射准备好的Rs到Ex去，
    //可以给出准备好的值
    void Launch(ControlBus & CBD){
        if(!CBD.isEmpty()) {
            bool flag = false;
            int Value1=0, Value2=0;
            Issue ExIssue;
            ExIssue.SetZero();
            for (int i = 0; i < Storage; i++) {
                if (RS_Next[i].isBusy && (!RS_Next[i].Pi.Rob) && (!RS_Next[i].Pj.Rob)) {
                    Value1 = RS_Next[i].Pi.Data;
                    Value2 = RS_Next[i].Pj.Data;
                    RS_Next[i].Pj.Rob=RS_Next[i].Pi.Rob=LazyFlag;
                    ExIssue = RS_Next[i].ExIssue;
                    flag = true;
                    break;
                }
            }
            //还有没有可执行的话，发射什么?
            CBD.RsLaunch(Value1, Value2, ExIssue);
            //阻塞了
            //发射准备好的RS到Ex里面
        }
        else {
            Issue Tmp;
            Tmp.SetZero();
            CBD.RsLaunch(Zero,Zero,Tmp);
        }
    }
    //接受哪一个ROB完成了 可能不一定是ROB头
    //然后根据完成的值 更新Rs里的值
    //同时缓存Commit的影响
    void CommitReceive(ControlBus & CBD) {
        //接收ROB的信息
        if (!CBD.isEmpty()) {
        //    std::cout<<"CommitSign"<<'\n';
            std::pair<int, std::pair<int, int>> CommitSign = CBD.CommitResult();
            for (int i = 0; i < Storage; i++) {
                if (RS_Prev[i].isBusy) {
                    if (RS_Prev[i].ExIssue.ROBid == CommitSign.first) {
                        RS_Next[i].isBusy = false;
                        RsSize_Next--;
                    }
                    if (RS_Prev[i].Pi.Rob!=0&&RS_Prev[i].Pi.Rob == CommitSign.first) {
                        RS_Next[i].Pi.Rob=0;
                        RS_Next[i].Pi.Data=CommitSign.second.first;
                    }
                    if (RS_Prev[i].Pj.Rob!=0&&RS_Prev[i].Pj.Rob == CommitSign.first) {
                        RS_Next[i].Pj.Rob=0;
                        RS_Next[i].Pj.Data=CommitSign.second.first;
                    }
                } else {}
            }
        }

    }
    //更新Issue来的
    //更新等待的
    //更新对应的
    void Renew(ControlBus & CBD){
            for (int i = 0; i < Storage; i++) {
                RS_Prev[i] = RS_Next[i];
            }
            RsSize_Prev = RsSize_Next;
           //阻塞则改回true
            //更新版本，包括更新ROB结果和Issue结果
    }
    void Clear(){
        RsSize_Next=0;
        memset(RS_Next,0,sizeof(RS_Next));
        //for(int i=0;i<Storage;i++) RS_Next[i].isBusy=false;
    }
};
class ExecutePart{
private:
    ExecuteTool execute;
public:
    ExecutePart()=default;
    ~ExecutePart()=default;
    void Execute(ControlBus & CBD){
            std::pair<Issue, std::pair<int, int>> RsSign = CBD.RsResult();
            if(RsSign.first.ROBid) {
                std::pair<int,int> ans = execute.Exec(RsSign.first, RsSign.first.pc, RsSign.second.first, RsSign.second.second);
                CBD.ExLaunch(RsSign.first.ROBid, ans.first,ans.second);
            }
            else{
                CBD.ExLaunch(Zero,Zero,Zero);
            }
    }
};
class PipeLing{
private:
    ControlBus Cdb;
    BinOrderCache Boc;
    StoreLoadBuffer Slb;
    ReOrderBuffer Rob;
    ReserveStation Rs;
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
        try {
            int i=0;
            while (true) {

                Rob.FunReceive(Cdb);
                Rs.CommitReceive(Cdb);
                Slb.CommitReceive(Cdb);
                reg.CommitReceive(Cdb);

                if(Cdb.isClear()){
                    Rob.Clear();
                    Rs.Clear();
                    Slb.Clear();
                    reg.Clear();
                    Boc.Clear();
                    Cdb.SetClear(False);
                    Cdb.SetStuck(False);
               //     std::cout<<"Clear"<<'\n';
                }


                Rob.Launch(Cdb);
                Rs.Launch(Cdb);
                Slb.Launch(Cdb);

                Boc.Fetch(Cdb);

                reg.Renew(Cdb);
                Rob.Renew(Cdb);
                Rs.Renew(Cdb);
                Slb.Renew(Cdb);
                Boc.Renew(Cdb);
                Cdb.Renew();

                Ex.Execute(Cdb);
                Boc.Decode(Cdb);
                Rob.IssueReceive(Cdb);
                Rs.IssueReceive(Cdb);
                Slb.IssueReceive(Cdb,i);
                reg.IssueReceive(Cdb);


                Rob.Commit(Cdb);
                reg.PcReceive(Cdb);
              //  std::cout<<i<<'\n';
                i++;
            }
        }catch(End){
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
