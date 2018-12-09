#include <iostream>
#include <string>
#include "md5.h"

using namespace std;

int main(int argc, char const *argv[])
{
    cout << "Please input the context to encript: ";
    string context;
    cin >> context;
    md5 code = md5(context);
    cout << endl << "The MD5 code is : " << endl << code.getCode() << endl;
    getchar();
    getchar();
    return 0;
}
