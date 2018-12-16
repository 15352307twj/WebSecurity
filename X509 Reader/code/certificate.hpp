#include <iostream>
#include <iomanip>
#include <string.h>
using namespace std;

FILE *file;
int printCount = 0;  // 表示输出的次数

pair<int, int> getTLV();
char* getHex(unsigned char c);
int getLength(unsigned char c);
char* getData(int length);
char* getAlgorithm(int length);
char* getWord(int length);
void printInfo(int time, char* data);
bool printDN(char *data); 
void printAlgorithm(char *data);

// 获取该数据块的 Type类型 Length长度 
pair<int, int> getTLV() {
    unsigned char type = fgetc(file);
    unsigned char len0 = fgetc(file);
    int len = getLength(len0); 

    bool isPrint = true;    // 信息是否输出
    char *buffer;      // 存储输出结果

    // char *temp;
    // temp = getHex(type);
    // std::cout << "Hex: " << temp << "   ";

    switch (type) {
        case 0x01:      // BOOLEAN
            break;
        case 0x02: {// INTEGER             
            buffer = getData(len);
            break;
        }
        case 0x03: { // BIT STRING
            buffer = getData(len);
            break;
        }
        case 0x04:      // OCTET STRING
            break;
        case 0x05:      // NULL
            strcpy(buffer, "NULL");
            break;
        case 0x06: {    // OBJECT IDENTIFIER
            buffer = getAlgorithm(len);
            break;
        }
        case 0x13: case 0x0C:      // PrintableString or UTF8String
            buffer = getWord(len);
            break;
        case 0x17: // UTCtime
            buffer = getWord(len);
            break;
        case 0x30: case 0x31: case 0xA0: {      // SEQUENCE or SET or Context
            isPrint = false;              
            int remain = len;
            while (remain > 0) 
                remain -= getTLV().first;
            break;
        }
        case 0xA3: {    // Context[3] 扩展部分            
            int remain = len;
            while (remain > 0) {
                fgetc(file);
                remain--;
            }
            break;
        }
        default:
            break;
    }

    if (isPrint) 
        printInfo(++printCount, buffer);
    return make_pair(len, 0);
}

// 将1个字节转换为2个16进制数表示
char* getHex(unsigned char c) {
    char *temp = new char[2];
    sprintf(temp, "%02X", c);
    return temp;
}

// 输出信息
bool printStatus = false; // 还未开始
void printInfo(int time, char *data) {
    switch (time) {
        case 1: {
            // 输出版本号
            cout << endl << endl <<  "# 待签名证书 tbsCertificate :" << endl;
            cout << left << setw(30) <<  "【版本】:" << "V" << char(data[1]+1) << endl;
            break;
        }
        case 2: {
            // 输出序列号
            cout << left << setw(30) <<  "【序列号】:" << data << endl;
            break;
        }
        case 3: {
            // 输出签名算法
            cout << left << setw(30) <<  "【签名算法】:";
            printAlgorithm(data);
            break;
        }
        case 4: {
            // 输出签名算法参数
            cout << left << setw(30) <<  "【签名算法所需参数】:" << data << endl;
            break;
        }
        case 5: {
            // 输出颁发者信息 - 信息类型
            if (printStatus == false) {// 第一次调用
                cout << left << setw(30) <<  "【颁发者】:";
                printStatus = true;
            }
            
            if (printDN(data))
                break;
            else {
                printCount += 2;
                time += 2;          // 说明这里并不是输出颁发者信息，而是输出有效期
                cout << endl;
                printStatus = false;    // 结束颁发者输出
            }                
        }
        case 6: {
            // 输出颁发者信息 - 信息数据
            if (printStatus) {
                cout << data << "  ";
                printCount -= 2; // 继续输出颁发者信息
                break;
            }
        }
        case 7: {
            // 输出有效期 - 起始
            cout << left << setw(30) <<  "【有效期】:" 
                << data[0] << data[1] << "年" << data[2] << data[3] << "月" << data[4] << data[5] << "日"
                << data[6] << data[7] << ":"  << data[8] << data[9] << ":"  << data[10] << data[11];
            break;
        }
        case 8: {
            // 输出有效期 - 终止
            cout << " 至 " 
                << data[0] << data[1] << "年" << data[2] << data[3] << "月" << data[4] << data[5] << "日"
                << data[6] << data[7] << ":"  << data[8] << data[9] << ":"  << data[10] << data[11]
                << "  (GMT)" << endl;
            break;
        }
        case 9: {
            // 输出持有者信息 - 信息类型
            if (printStatus == false) {// 第一次调用
                cout << left << setw(30) <<  "【持有者】:";
                printStatus = true;
            }
            
            if (printDN(data))
                break;
            else {
                printCount += 2;
                time += 2;              // 说明这里并不是输出持有者信息，而是输出公钥
                cout << endl;
                printStatus = false;    // 结束持有者输出
            }   
        }
        case 10 : {
            // 输出颁发者信息 - 信息数据
            if (printStatus) {
                cout << data << "  ";
                printCount -= 2;        // 继续输出持有者信息
                break;
            }
        }
        case 11: {
            cout << left << setw(30) <<  "【公钥加密算法】:";
            printAlgorithm(data);
            break;
        }
        case 12: {
            cout << left << setw(30) <<  "【公钥加密算法参数】:" << data << endl;
            break;
        }
        case 13: {
            cout << left << setw(30) <<  "【公钥】:" << data << endl;
            break;
        }
        case 14: {
            cout << left << setw(30) <<  "【扩展部分】:" << "略" << endl;
            break;
        }
        case 15: {
            // 输出签名算法
            cout << endl << endl <<  "# 签名算法 signatureAlgorithm :" << endl;
            cout << left << setw(30) <<  "【签名算法】:";
            printAlgorithm(data);
            break;
        }
        case 16: {
            // 输出签名算法参数
            cout << left << setw(30) <<  "【签名算法所需参数】:" << data << endl;
            break;
        }
        case 17: {
            // 输出签名值
            cout << endl << endl <<  "# 签名值 signatureValue :" << endl;
            cout << left << setw(30) <<  "【签名值】:" << data << endl;
        }
        default:
            break;
    }
}

// 获取数据块的真实长度
int getLength(unsigned char len0) {
    if (len0 > 0x80) {              // 判断长度是否大于127(最高为是否为1)
        int blockNum = len0 - 0x80; // 若是，获取后面长度使用的字节的数量
        unsigned char temp;
        int len = 0;
        for (int i = 0; i < blockNum; i++)
            len = len * 256 + fgetc(file);
        return len;
    } else
        return len0;
}

// 获取十六进制数据
char* getData(int length) {
    char *buffer = new char[length * 2];
    unsigned char byte;
    strcpy(buffer, "");
    for (int i = 0; i < length; i++) {
        byte = fgetc(file);
        strcat(buffer, getHex(byte));
        // cout << buffer << endl;
    }
    return buffer;
}

// 获取签名算法
char* getAlgorithm(int length) {
    char tempBuffer[10];

    // 处理第一个字符
    unsigned char firstLetter = fgetc(file);
    int x = firstLetter / 40; 
    int y = firstLetter % 40;
    char *buffer = new char[30];
    strcpy(buffer, "");
    sprintf(tempBuffer, "%d", x);
    strcat(buffer, tempBuffer);
    strcat(buffer, ".");
    sprintf(tempBuffer, "%d", y);
    strcat(buffer, tempBuffer);

    // 处理后面的字符
    unsigned char temp;
    int num = 0;
    for (int i = 0; i < length-1; i++) {
        temp = fgetc(file);
        if (temp & 0x80) 
            num = (num << 7) + (temp & 0x7f);
        else {
            num = (num << 7) + (temp & 0x7f);
            strcat(buffer, ".");
            sprintf(tempBuffer, "%d", num);
            strcat(buffer, tempBuffer);
            num = 0;
        }        
    }
    return buffer;
}

// 获取词语
char* getWord(int length) {
    char *buffer = new char[length + 1];
    unsigned char byte;
    for (int i = 0; i < length; i++) {
        byte = fgetc(file);
        buffer[i] = byte;
    }
    buffer[length] = '\0';
    return buffer;
}

// 输出算法
void printAlgorithm(char *data) {
    if (strcmp(data, "1.2.840.10040.4.1") == 0)
        cout << "DSA" << endl;
    else if (strcmp(data, "1.2.840.10040.4.3") == 0)
        cout << "sha1DSA" << endl;
    else if (strcmp(data, "1.2.840.113549.1.1.1") == 0)
        cout << "RSA" << endl;
    else if (strcmp(data, "1.2.840.113549.1.1.2") == 0)
        cout << "md2RSA" << endl;
    else if (strcmp(data, "1.2.840.113549.1.1.3") == 0)
        cout << "md4RSA" << endl;
    else if (strcmp(data, "1.2.840.113549.1.1.4") == 0)
        cout << "md5RSA" << endl;
    else if (strcmp(data, "1.2.840.113549.1.1.5") == 0)
        cout << "sha1RSA" << endl;
    else if (strcmp(data, "1.3.14.3.2.29") == 0)
        cout << "sha1RSA" << endl;
    else if (strcmp(data, "1.2.840.113549.1.1.13") == 0)
        cout << "sha512RSA" << endl;
    else if (strcmp(data, "1.2.840.113549.1.1.11") == 0)
        cout << "sha256RSA" << endl;
    else if (strcmp(data, "1.2.840.10045.2.1") == 0)
        cout << "ECC(256Bits)" << endl;
    else 
        cout << "Unknow, OID = " << data << endl;
}

// 输出信息
bool printDN(char *data) {
    cout ; 
    if (strcmp(data, "2.5.4.6") == 0) 
        cout << left << setw(13) << "国名："; 
    else if (strcmp(data, "2.5.4.8") == 0) 
        cout << endl << left << setw(30) <<  " " << left << setw(13) << "洲/省名："; 
    else if (strcmp(data, "2.5.4.7") == 0) 
        cout << endl << left << setw(30) <<  " " << left << setw(13) << "地理位置：";  
    else if (strcmp(data, "2.5.4.10") == 0) 
        cout << endl << left << setw(30) <<  " " << left << setw(13) << "机构名：";  
    else if (strcmp(data, "2.5.4.11") == 0) 
        cout << endl << left << setw(30) <<  " " << left << setw(13) << "机构单元名称：";   
    else if (strcmp(data, "2.5.4.3") == 0) 
        cout << endl << left << setw(30) <<  " " << left << setw(13) << "通用名称：";  
    else
        return false;

    return true;
}