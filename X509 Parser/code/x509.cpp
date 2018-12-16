#include <iostream>
#include "certificate.hpp"

using namespace std;

int main() {
    char *filename= new char[256];
    cout << "请输入证书的文件名: ";
    cin >> filename;

    file = fopen(filename, "rb");
    if (file == NULL) {
        cout << "找不到该文件...\n";
        getchar();
        return 0;
    }

    getTLV();
    fclose(file);
    return 0;
}   

//g++ x509.cpp certificate.hpp -o x509.exe -std=c++11 -fexec-charset=gbk