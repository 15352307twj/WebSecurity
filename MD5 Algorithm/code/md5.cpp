#include "md5.h"

// public
md5::md5(string _init) {
    text = _init;           // 源字符串
    length = text.length(); // 源字符串长度
    code = "";              // 初始md5值为空
}

string md5::getCode() {
    fill();
    loop();
    code = converse(A) + converse(B) + converse(C) + converse(D); 
    return code;
}

string md5::converse(unsigned int num) {
    unsigned int tmp;
    string result;
    char buf[4];
    for (int i = 0; i < 4; i++) {
        tmp = num & 0xff;
        sprintf(buf, "%02X", tmp); 
        num = num >> 8;
        result += buf;
    }
    return result;
}

// private:
void md5::fill() {
    int fillBit = 448 - ((length * 8) % 512);           // 需要补位的位数
    int fillByte = fillBit == 0 ? 512 : fillBit / 8;    // 需要补位的字节数

    text += char(0x80);                     // 先补上10000000
    for (int i = 0; i < fillByte-1; i++)
        text += char(0x00);                 // 往后继续补00000000

    unsigned int temp = length * 8; // 长度以字节为单位
    for (int i = 0 ; i < 8; i++) {
        text += char(temp & 0xff);  // 补充64位/8字节的信息长度，每次填充取低8位
        temp = temp >> 8;
    }      
}

void md5::loop() {
    fillLength = text.length();
    unsigned int loopNum = fillLength / 64;                 // 每512位（64字节）为一组

    for (unsigned int i = 0; i < loopNum; i++) {     
        unsigned int *seed = new unsigned int[4] {A, B, C, D};   
        AccLoop(seed, text.substr(i * 64, 64));
        A += seed[0];
        B += seed[1];
        C += seed[2];
        D += seed[3];
    }
}

void md5::AccLoop(unsigned int *seed, string group) {
    int indexArr[4][16] = { 
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
        { 1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12 },
        { 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2 },
        { 0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9 }
    };
    int shiftArr[4][4] = {
        { 7, 12, 17, 22 },
        { 5, 9, 14, 20 },
        { 4, 11, 16, 23 },
        { 6, 10, 15, 21 }
    };

    // Ti
    unsigned int T[64];
    for (int i = 0; i < 64; i++) {
        T[i] = (unsigned int)(0x100000000UL * abs(sin((double)(i + 1))));
    }

    // 将64字节划分为16组4字节(32位)，转换为数字
    unsigned int M[16];
    for (int i = 0; i < 16; i++) {
        string sub = group.substr(i * 4, 4);
        M[i] = (((sub[3]&0xff) << 24) + ((sub[2]&0xff) << 16) + ((sub[1]&0xff) << 8) + (sub[0]&0xff));
    }

    unsigned int i1 = seed[0], 
                i2 = seed[1], 
                i3 = seed[2], 
                i4 = seed[3], 
                temp;

    // 计算开始
    for (int k = 0 ; k < 4; k++)        // 四轮循环
        for (int i = 0; i < 16; i++) {  // 每轮循环执行16次函数
            switch (k) {
                case 0:
                    temp = F(i2, i3, i4);
                    break;
                case 1:
                    temp = G(i2, i3, i4);
                    break;
                case 2:
                    temp = H(i2, i3, i4);
                    break;
                case 3:
                    temp = I(i2, i3, i4);
                    break;
            }
            temp += i1 +  M[indexArr[k][i]] + T[k * 16 + i];
            temp = SHIFT_LEFT(temp, shiftArr[k][i % 4]);
            temp = temp + i2;
            
            i1 = i4;
            i4 = i3;
            i3 = i2;
            i2 = temp;
        }

    seed[0] = i1;
    seed[1] = i2;
    seed[2] = i3;
    seed[3] = i4;
}