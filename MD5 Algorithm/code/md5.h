#ifndef MD5_H
#define MD5_H

#include <string>
#include <cmath>
using namespace std;

#define F(x, y, z) (x & y) | ((~x) & z)
#define G(x, y, z) (x & z) | (y & (~z))
#define H(x, y, z) x ^ y ^ z
#define I(x, y, z) y ^ (x | (~z))

#define SHIFT_LEFT(a, s) ((a << s) | (a >> (32 - s)))

class md5{
    private:
        string text;                // 字符串
        string code;                // md5值
        unsigned int length;        // 源字符串长度
        unsigned int fillLength;    // 补全后字符串长度
        unsigned int A = 0x67452301, 
            B = 0xEFCDAB89, 
            C = 0x98BADCFE, 
            D = 0x10325476;         // 小端类型

    public:
        md5(string _init);          // 构造函数 
        string getCode();           // 获取md5值

    private:
        void fill();                                    // 填充函数
        void loop();                                    // 512位分组处理函数
        string converse(unsigned int num);              // 转换为小端序函数
        void AccLoop(unsigned int *seed, string group); // 子分组计算函数
};

#endif