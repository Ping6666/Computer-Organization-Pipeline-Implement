#include <cmath>    //需要使用到pow()
#include <cstring>  //需要使用到字串處理
#include <fstream>  //讀取資料
#include <iostream> //輸入輸出
#include <vector>   //MIPS指令存放位置
//備註：不使用using namespace std是因為這個很髒

int main()
{
    //輸入檔案名稱
    std::string InFile[4] = {"General.txt", "Datahazard.txt", "Lwhazard.txt", "Branchhazard.txt"};
    //輸出檔案名稱
    std::string OutFile[4] = {"genResult.txt", "dataResult.txt", "loadResult.txt", "branchResult.txt"};

    for (int i = 0; i < 4; i++) //i<4
    {
        //按照順序輸入檔案
        std::fstream inputcode;
        inputcode.open(InFile[i], std::ios::in);
        //宣告MIPS指令存放位置
        std::vector<std::string> codevector;
        std::string tmpstring = "";
        while (inputcode.peek() != EOF)
        {
            inputcode >> tmpstring;          //從檔案中取出MIPS指令
            codevector.push_back(tmpstring); //加入vector之中
        }
        //宣告輸出檔案
        std::fstream outputcode;
        //先清空檔案，接者開始輸出
        outputcode.open(OutFile[i], std::ios::out | std::ios::trunc);

        //初始化register中的數值
        int regtable[10] = {0, 9, 5, 7, 1, 2, 3, 4, 5, 6};
        //初始化memory中的數值
        int memtable[5] = {5, 9, 4, 8, 7};
        //宣告memory的位置，簡化之後輸出的麻煩
        std::string memnumtable[5] = {"0x00", "0x04", "0x08", "0x0C", "0x10"};
        //此為add,sub,and,or,slt分別的function number
        std::string ifuntable[5] = {"100000", "100010", "100100", "100101", "101010"};
        //此為lw,sw,addi,andi,beq分別的opcode
        std::string ropcodetable[5] = {"100011", "101011", "001000", "001100", "000100"};

        //stall表需要暫停幾個CC，state表是否正在stall
        int stall = 0, state = 0;
        //shift表需要跳幾個指令，shiftstate表是否正在shiftstate，shiftchange表是否shift剛結束
        int shift = 0, shiftstate = 0, shiftchange = 0;
        //CCmax表原本預估最大CC
        int CCmax = 5 + (codevector.size() - 1);
        //ReadData1,ReadData2表ID/EX中的暫存資料，sign_ext表MEM/WB中的immediate資訊(不一定要用到)
        int ReadData1 = 0, ReadData2 = 0, sign_ext = 0;
        //Rs,Rt,Rd表ID/EX中的記憶體位置
        int Rs = 0, Rt = 0, Rd = 0;
        //ALUout0表EX/MEM中的計算結果，WriteData表EX/MEM中的暫存資料，RtRd0表EX/MEM中的目標記憶體位置
        int ALUout0 = 0, WriteData = 0, RtRd0 = 0;
        //ReadData表MEM/WB中的暫存資料，ALUout1表MEM/WB中的計算結果，RtRd1表MEM/WB中的目標記憶體位置
        int ReadData = 0, ALUout1 = 0, RtRd1 = 0;
        //idxop表opcode的不同階段的I-type指令，idxfunc表function的不同階段的R-type指令
        int idxop[3] = {0}, idxfunc[3] = {0};
        //Controlsignals表Control signals的不同階段的狀態
        std::string Controlsignals[3] = {"000000000", "000000000", "000000000"};
        //CC的迴圈數量受到CCmax,stall,shift影響
        for (int CC = 0; CC < CCmax + stall - shift; CC++)
        {
            //CC輸出
            outputcode << "CC " << CC + 1 << ":\n";
            //Registers輸出
            outputcode << "\nRegisters:\n";
            for (int j = 0; j < 10; j++)
            {
                outputcode << "$" << j << ": " << regtable[j] << "\n";
            }
            //Data memory輸出
            outputcode << "\nData memory:\n";
            for (int j = 0; j < 5; j++)
            {
                outputcode << memnumtable[j] << ": " << memtable[j] << "\n";
            }
            //IF/ID輸出
            outputcode << "\nIF/ID :\n";
            outputcode << "PC		" << (CC + 1) * 4 << "\n";
            std::string tmpstr;
            //從vector中取得當前指令
            if (CC - stall + shift < codevector.size())
            {
                tmpstr = codevector[CC - stall + shift];
            }
            else
            {
                tmpstr = "00000000000000000000000000000000";
            }
            outputcode << "Instruction	" << tmpstr << "\n";
            //ID/EX輸出
            outputcode << "\nID/EX :\n";
            outputcode << "ReadData1	" << ReadData1 << "\n";
            outputcode << "ReadData2	" << ReadData2 << "\n";
            outputcode << "sign_ext	" << sign_ext << "\n";
            outputcode << "Rs		" << Rs << "\n";
            outputcode << "Rt		" << Rt << "\n";
            outputcode << "Rd		" << Rd << "\n";
            outputcode << "Control signals	" << Controlsignals[0] << "\n";
            //EX/MEM輸出
            outputcode << "\nEX/MEM :\n";
            outputcode << "ALUout		" << ALUout0 << "\n";
            outputcode << "WriteData	" << WriteData << "\n";
            outputcode << "Rt/Rd		" << RtRd0 << "\n";
            outputcode << "Control signals	" << Controlsignals[1].substr(4, 5) << "\n";
            //MEM/WB輸出
            outputcode << "\nMEM/WB :\n";
            outputcode << "ReadData	" << ReadData << "\n";
            outputcode << "ALUout		" << ALUout1 << "\n";
            outputcode << "Rt/Rd		" << RtRd1 << "\n";
            outputcode << "Control signals	" << Controlsignals[2].substr(7, 2) << "\n";
            //END部分
            for (int j = 0; j < 65; j++)
            {
                outputcode << "=";
            }
            outputcode << "\n";
            //此階段輸出END

            if (state > 0)
            {
                state -= 1;
                //此表正在state，所以不處理stage2的事情

                //part MEM/WB
                if (idxfunc[2] == -1 && idxop[2] == 0) //lw
                {
                    int idx = ALUout1 / 4;
                    ReadData = memtable[idx];
                    regtable[RtRd1] = ReadData;
                    //從memory中取得資料
                }
                else if (idxfunc[2] == -1 && idxop[2] == 1) //sw
                {
                    memtable[RtRd1] = ALUout1;
                    //將資料放入memory
                }
                else if (idxfunc[2] != -1 && idxop[2] != 4) //other except beq
                {
                    regtable[RtRd1] = ALUout1;
                    //將資料放入register
                }
                //資料前後傳遞階段(詳見每個變數定義)
                ALUout1 = ALUout0;
                RtRd1 = RtRd0;
                idxop[2] = idxop[1];
                idxfunc[2] = idxfunc[1];
                //ReadData校正
                if (idxfunc[2] == -1 && idxop[2] == 0) //lw
                {
                    int idx = ALUout1 / 4;
                    ReadData = memtable[idx];
                }
                else if (idxfunc[2] != -1 && idxop[2] != 4) //other except beq
                {
                    ReadData = 0;
                }
                //資料前後傳遞階段(詳見每個變數定義)
                Controlsignals[2] = Controlsignals[1];

                //part EX/MEM
                //資料前後傳遞階段(詳見每個變數定義)
                idxop[1] = idxop[0];
                idxfunc[1] = idxfunc[0];
                if (idxop[1] == -1 && idxfunc[1] == -1) //nop
                {
                    RtRd0 = 0;
                    ALUout0 = 0;
                    //沒有動作歸零變數
                }
                else if (idxop[1] == -1) //add,sub,and,or,slt
                {
                    RtRd0 = Rd;
                    if (idxfunc[1] == 0) //add
                    {
                        ALUout0 = ReadData1 + ReadData2;
                        //ALU做相加
                    }
                    else if (idxfunc[1] == 1) //sub
                    {
                        ALUout0 = ReadData1 - ReadData2;
                        //ALU做相減
                    }
                    else if (idxfunc[1] == 2) //and
                    {
                        //ALU做and
                        int tmp1 = ReadData1, tmp2 = ReadData2;
                        int idx = 0;
                        ALUout0 = 0;
                        while (tmp1 != 0 || tmp2 != 0)
                        {
                            if (tmp1 % 2 == 1 && tmp2 % 2 == 1)
                            {
                                ALUout0 += pow(2, idx);
                            }
                            idx += 1;
                            tmp1 /= 2;
                            tmp2 /= 2;
                        }
                    }
                    else if (idxfunc[1] == 3) //or
                    {
                        //ALU做or
                        int tmp1 = ReadData1, tmp2 = ReadData2;
                        int idx = 0;
                        ALUout0 = 0;
                        while (tmp1 != 0 || tmp2 != 0)
                        {
                            if (tmp1 % 2 == 1 || tmp2 % 2 == 1)
                            {
                                ALUout0 += pow(2, idx);
                            }
                            idx += 1;
                            tmp1 /= 2;
                            tmp2 /= 2;
                        }
                    }
                    else if (idxfunc[1] == 4) //slt
                    {
                        //ALU做slt
                        if (ReadData1 < ReadData2)
                        {
                            ALUout0 = 1;
                        }
                        else
                        {
                            ALUout0 = 0;
                        }
                    }
                }
                else if (idxfunc[1] == -1) //lw,sw,addi,andi,beq
                {
                    RtRd0 = Rt;
                    if (idxop[1] == 0 || idxop[1] == 1 || idxop[1] == 2) //lw,sw,addi
                    {
                        ALUout0 = ReadData1 + sign_ext;
                        //ALU計算目標(位置)或是正確答案(數值)
                    }
                    else if (idxop[1] == 3) //andi
                    {
                        //ALU做and
                        int tmp1 = ReadData1, tmp2 = sign_ext;
                        int idx = 0;
                        ALUout0 = 0;
                        while (tmp1 != 0 || tmp2 != 0)
                        {
                            if (tmp1 % 2 == 1 && tmp2 % 2 == 1)
                            {
                                ALUout0 += pow(2, idx);
                            }
                            idx += 1;
                            tmp1 /= 2;
                            tmp2 /= 2;
                        }
                    }
                    else if (idxop[1] == 4)
                    {
                        ALUout0 = ReadData1 - ReadData2;
                        //ALU做beq檢查
                        if (ALUout0 == 0 && shiftstate == 0)
                        {
                            shift += sign_ext - 1;
                            shiftstate = 1;
                            shiftchange = 1;
                            //設定shift相關狀態
                            idxop[0] = -1;
                            idxfunc[0] = -1;
                            Controlsignals[0] = "000000000";
                            Rs = 0;
                            Rt = 0;
                            Rd = 0;
                            sign_ext = 0;
                            ReadData1 = 0;
                            ReadData2 = 0;
                            //清空stage2狀態，並且初始化
                        }
                    }
                }
                else
                {
                    RtRd0 = 0;
                    ALUout0 = 0;
                    //避免錯誤，歸零變數
                }
                //資料前後傳遞階段(詳見每個變數定義)
                WriteData = ReadData2;
                Controlsignals[1] = Controlsignals[0];
            }
            else
            {
                //part MEM/WB
                if (idxfunc[2] == -1 && idxop[2] == 0) //lw
                {
                    int idx = ALUout1 / 4;
                    ReadData = memtable[idx];
                    regtable[RtRd1] = ReadData;
                    //從memory中取得資料
                }
                else if (idxfunc[2] == -1 && idxop[2] == 1) //sw
                {
                    memtable[RtRd1] = ALUout1;
                    //將資料放入memory
                }
                else if (idxfunc[2] != -1 && idxop[2] != 4) //other except beq
                {
                    regtable[RtRd1] = ALUout1;
                    //將資料放入register
                }
                //資料前後傳遞階段(詳見每個變數定義)
                ALUout1 = ALUout0;
                RtRd1 = RtRd0;
                idxop[2] = idxop[1];
                idxfunc[2] = idxfunc[1];
                //ReadData校正
                if (idxfunc[2] == -1 && idxop[2] == 0) //lw
                {
                    int idx = ALUout1 / 4;
                    ReadData = memtable[idx];
                }
                else if (idxfunc[2] != -1 && idxop[2] != 4) //other except beq
                {
                    ReadData = 0;
                }
                //資料前後傳遞階段(詳見每個變數定義)
                Controlsignals[2] = Controlsignals[1];

                //part EX/MEM
                //資料前後傳遞階段(詳見每個變數定義)
                idxop[1] = idxop[0];
                idxfunc[1] = idxfunc[0];
                if (idxop[1] == -1 && idxfunc[1] == -1) //nop
                {
                    RtRd0 = 0;
                    ALUout0 = 0;
                    //沒有動作歸零變數
                }
                else if (idxop[1] == -1) //add,sub,and,or,slt
                {
                    RtRd0 = Rd;
                    if (idxfunc[1] == 0) //add
                    {
                        ALUout0 = ReadData1 + ReadData2;
                        //ALU做相加
                    }
                    else if (idxfunc[1] == 1) //sub
                    {
                        ALUout0 = ReadData1 - ReadData2;
                        //ALU做相減
                    }
                    else if (idxfunc[1] == 2) //and
                    {
                        //ALU做and
                        int tmp1 = ReadData1, tmp2 = ReadData2;
                        int idx = 0;
                        ALUout0 = 0;
                        while (tmp1 != 0 || tmp2 != 0)
                        {
                            if (tmp1 % 2 == 1 && tmp2 % 2 == 1)
                            {
                                ALUout0 += pow(2, idx);
                            }
                            idx += 1;
                            tmp1 /= 2;
                            tmp2 /= 2;
                        }
                    }
                    else if (idxfunc[1] == 3) //or
                    {
                        //ALU做or
                        int tmp1 = ReadData1, tmp2 = ReadData2;
                        int idx = 0;
                        ALUout0 = 0;
                        while (tmp1 != 0 || tmp2 != 0)
                        {
                            if (tmp1 % 2 == 1 || tmp2 % 2 == 1)
                            {
                                ALUout0 += pow(2, idx);
                            }
                            idx += 1;
                            tmp1 /= 2;
                            tmp2 /= 2;
                        }
                    }
                    else if (idxfunc[1] == 4) //slt
                    {
                        //ALU做slt
                        if (ReadData1 < ReadData2)
                        {
                            ALUout0 = 1;
                        }
                        else
                        {
                            ALUout0 = 0;
                        }
                    }
                }
                else if (idxfunc[1] == -1) //lw,sw,addi,andi,beq
                {
                    RtRd0 = Rt;
                    if (idxop[1] == 0 || idxop[1] == 1 || idxop[1] == 2) //lw,sw,addi
                    {
                        ALUout0 = ReadData1 + sign_ext;
                        //ALU計算目標(位置)或是正確答案(數值)
                    }
                    else if (idxop[1] == 3) //andi
                    {
                        //ALU做and
                        int tmp1 = ReadData1, tmp2 = sign_ext;
                        int idx = 0;
                        ALUout0 = 0;
                        while (tmp1 != 0 || tmp2 != 0)
                        {
                            if (tmp1 % 2 == 1 && tmp2 % 2 == 1)
                            {
                                ALUout0 += pow(2, idx);
                            }
                            idx += 1;
                            tmp1 /= 2;
                            tmp2 /= 2;
                        }
                    }
                    else if (idxop[1] == 4)
                    {
                        ALUout0 = ReadData1 - ReadData2;
                        //ALU做beq檢查
                        if (ALUout0 == 0 && shiftstate == 0)
                        {
                            shift += sign_ext - 1;
                            shiftstate = 1;
                            shiftchange = 1;
                            //設定shift相關狀態
                            idxop[0] = -1;
                            idxfunc[0] = -1;
                            Controlsignals[0] = "000000000";
                            Rs = 0;
                            Rt = 0;
                            Rd = 0;
                            sign_ext = 0;
                            ReadData1 = 0;
                            ReadData2 = 0;
                            //清空stage2狀態，並且初始化
                        }
                    }
                }
                else
                {
                    RtRd0 = 0;
                    ALUout0 = 0;
                    //避免錯誤，歸零變數
                }
                //資料前後傳遞階段(詳見每個變數定義)
                WriteData = ReadData2;
                Controlsignals[1] = Controlsignals[0];

                if (shiftstate == 1)
                {
                    shiftstate = 0;
                    //此表正在shift
                }
                else
                {
                    //part ID/EX
                    //取出opcode與function
                    std::string opcode = tmpstr.substr(0, 6);
                    std::string func = tmpstr.substr(26, 6);
                    if (tmpstr == "00000000000000000000000000000000") //nop
                    {
                        idxop[0] = -1;
                        idxfunc[0] = -1;
                        Controlsignals[0] = "000000000";
                        //設定無動作的情況
                    }
                    else if (opcode == "000000") //R-type
                    {
                        idxop[0] = -1;
                        Controlsignals[0] = "110000010";
                        for (int j = 0; j < 5; j++)
                        {
                            if (func == ifuntable[j])
                            {
                                idxfunc[0] = j;
                                //查表判斷R-type指令
                                break;
                            }
                        }
                    }
                    else //I-type or others
                    {
                        idxfunc[0] = -1;
                        for (int j = 0; j < 5; j++)
                        {
                            if (opcode == ropcodetable[j])
                            {
                                idxop[0] = j;
                                //查表判斷I-type指令or others，並且設定Controlsignals
                                if (j == 0)
                                {
                                    Controlsignals[0] = "000101011"; //lw
                                }
                                else if (j == 1)
                                {
                                    Controlsignals[0] = "000100100"; //sw
                                }
                                else if (j == 4)
                                {
                                    Controlsignals[0] = "001010000"; //beq
                                }
                                else if (j == 2)
                                {
                                    Controlsignals[0] = "011100010"; //addi
                                }
                                else if (j == 3)
                                {
                                    Controlsignals[0] = "000100010"; //andi
                                }
                                break;
                            }
                        }
                    }
                    //取出Rs,Rt,Rd,sign_ext
                    Rs = std::stoi(tmpstr.substr(6, 5).c_str(), nullptr, 2);
                    Rt = std::stoi(tmpstr.substr(11, 5).c_str(), nullptr, 2);
                    Rd = std::stoi(tmpstr.substr(16, 5).c_str(), nullptr, 2);
                    sign_ext = std::stoi(tmpstr.substr(16, 16).c_str(), nullptr, 2);
                    if (Controlsignals[0] == "000000000")
                    {
                        ReadData1 = 0;
                        ReadData2 = 0;
                    }
                    else
                    {
                        bool check = 0;
                        if (Rs == RtRd0)
                        {
                            ReadData1 = ALUout0;
                            check = 1;
                            //此表可能會遇到Datahazard
                        }
                        else if (Rs == RtRd1 && shiftchange == 0)
                        {
                            ReadData1 = ALUout1;
                            //取得正確資料，且沒有剛結束shift
                        }
                        else
                        {
                            ReadData1 = regtable[Rs];
                            shiftchange = 0;
                            //重設shiftchange狀態，為了避免讀錯資料
                        }
                        if (Rt == RtRd0)
                        {
                            ReadData2 = ALUout0;
                            check = 1;
                            //此表可能會遇到Datahazard
                        }
                        else if (Rt == RtRd1 && shiftchange == 0)
                        {
                            ReadData2 = ALUout1;
                            //取得正確資料，且沒有剛結束shift
                        }
                        else
                        {
                            ReadData2 = regtable[Rt];
                            shiftchange = 0;
                            //重設shiftchange狀態，為了避免讀錯資料
                        }
                        if (check == 1 && state == 0)
                        {
                            if (Controlsignals[1] == "000101011" || Controlsignals[1] == "000100100")
                            {
                                stall += 1;
                                state = 1;
                                //若指令為lw,sw則設定stall狀態
                            }
                        }
                    }
                }
            }
        }
        //關閉檔案
        inputcode.close();
        outputcode.close();
    }

    return 0;
}
