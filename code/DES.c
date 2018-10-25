#include <stdio.h>
#include <string.h>
#include "table.h"

// 使用typedef绑定数据类型
typedef char Elem;

//------------------二进制位串 和 字符串 的相互转换-------------------------
void Char2Bit(Elem ch[8], int bit[64]) {
	int i, j;
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			bit[8 * (i + 1) - (j + 1)] = (ch[i] >> j) & 1;
}

void Bit2Char(int bit[64], Elem ch[8]) {
	int i, j;
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			ch[i] = (ch[i] << 1) + bit[8 * i + j];
}


//--------------------------16个子密钥生成-------------------------------
void CreateSubKey(int key[64], int subKeys[16][48]) {
	int data[56]; // 用于存储56位数据
	int i = 0;

	// PC1置换
	for (i = 0; i < 56; i++)
		data[i] = key[PC_1[i]];

	// 生成16个子密钥
	for (i = 0; i < 16; i++) {
		int j;

		// 左移
		int time = left_shift[i],
		    temp1[2] = {data[0], data[1]}, 
		    temp2[2] = {data[28], data[29]}; 
		for (j = 0; j < 28 - time; j++) {
			data[j] = data[j + time];
			data[28 + j] = data[28 + j + time];
		}
		for (j = 1; j <= time; j++) {
			data[28 - j] = temp1[time - j];
			data[56 - j] = temp2[time - j];
		}

		// PC2置换
		for (j = 0; j < 48; j++)
			subKeys[i][j] = data[PC_2[j]];
	}
}

//------------------------密码函数f--------------------------------

void f (int right[32], int subkey[48]) {
	// 右32位进行扩展 & 异或运算
	int i;
	int extend[48];
	for (i = 0; i < 48; i++)
		extend[i] = (right[extend_table[i]] ^ subkey[i]);

	/* 分成8个6位块，通过S表产生4位输出，共32位
	把6位输入中的第1, 6位取出来合成一个两位的二进制数 x ，作为行数（0~3）；
	把6位输入的中间4位构成另外一个二进制数 y，作为列数（0~15）；
	查出 Si 表中 x 行 y 列所对应的整数，将该整数转换为一个 4 位的二进制数。
	*/
	int temp[32];
	int x, y, row, col, target;
	for (i = 0; i < 8; i++) {
		x = 6 * i;
		row = (extend[x] << 1) + extend[x + 5];
		col = (extend[x + 1] << 3) 
				+ (extend[x + 2] << 2)
				+ (extend[x + 3] << 1)
				+ extend[x + 4];
		target = S[i][row][col];

		y = 4 * i;
		temp[y] = (target & 8) >> 3;
		temp[y + 1] = (target & 4) >> 2;
		temp[y + 2] = (target & 2) >> 1;
		temp[y + 3] = (target & 1);
	}

	// p置换
	for (i = 0; i < 32; i++)
		right[i] = temp[p_table[i]];
}

//-------------------------分组加密/解密--------------------------------
void CryptologyBlock(Elem in[8], Elem out[8], int subKeys[16][48], int isDecode) {
	int temp[64], inBit[64];
	Char2Bit(in, temp);
	int i;

	
	// IP 初始置换
	for (i = 0; i < 64; i++)
		inBit[i] = temp[IP_table[i]];

	// 分成左右各32位，进行16次迭代
	int left[32], right[32], swap[32];
	for (i = 0; i < 32; i++) {
		left[i] = inBit[i];
		right[i] = inBit[32 + i];
	}

	for (i = 0; i < 16; i++) {
		// right = f(right, subKeys[i]) xor left
		// left = right
		int j;
		for (j = 0; j < 32; j++)
			swap[j] = right[j];

		// 根据需求：加密/解密，确定使用哪一个子密钥
		int index;
		index = (isDecode == 0) ? i : (15 - i);
		f(right, subKeys[index]);

		for (j = 0; j < 32; j++) {
			right[j] ^= left[j];
			left[j] = swap[j];
		}
	}

	// left & right互换合并，进行IIP，输出
	for (i = 0; i < 32; i++) {
		temp[i] = right[i];
		temp[32 + i] = left[i];
	}
	for (i = 0; i < 64; i++)
		inBit[i] = temp[IIP_table[i]];

	Bit2Char(inBit, out);
}

// ----------------------------DES算法整体框架----------------------------
int file_rw(char *inFile, char *outFile, char *keyText, int isDecode) {
	FILE *in, *out;
	int count;
	Elem inBuffer[8], outBuffer[8], keyBuffer[8];
	int bitKey[64];
	int subKeys[16][48];

	// 打开文件
	in = fopen(inFile, "rb");
	out = fopen(outFile, "wb"); 
	if (in == NULL) {
		printf("There is no input_file\n");
		return 0;
	}
	if (out == NULL) {
		printf("Fail to create output_file\n");
		return 0;
	}

	// 先将8字节的密钥转换成64位的格式，再生成16个子密钥
	memcpy(keyBuffer, keyText, 8);
	Char2Bit(keyBuffer, bitKey);
	CreateSubKey(bitKey, subKeys);

	while (!feof(in)) {
		// 读入8字节. 若不足8字节，补足8字节
		count = fread(inBuffer, sizeof(char), 8, in);
		if (count > 0 && count < 8) 
			memset(inBuffer + count, '\0', 8 - count);	
		else if (count == 0)
			break;

		// 分组加密/解密
		CryptologyBlock(inBuffer, outBuffer, subKeys, isDecode);

		// 输出8字节结果
		fwrite(outBuffer, sizeof(char), 8, out);
	}

	fclose(in);
	fclose(out);
	return 1;
} 

int main() {
	char inText[64], outText[64], act[64];
	int isDecode;

	printf("Encrypt or Decrypt ? \n  e for Encrypt; d for Decrypt; \n  Enter : ");
	scanf("%s", act);
	switch (act[0]) {
		case 'e' :
			isDecode = 0;
			break;
		case 'd':
			isDecode = 1;
			break;
		default:
			printf("Wrong Input. \n");
			getchar();
			return 0;
	}
	printf("Please enter the input_file's name: ");
	scanf("%s", inText);
	printf("Please enter the output_file's name: ");
	scanf("%s", outText);

	char key[8];
	printf("Please enter the 8-bit key:");
	scanf("%s", key);

	file_rw(inText, outText, key, isDecode);
	if (isDecode == 0)
		printf("Encrypt Finish...\n");
	else
		printf("Decoding Finish...\n");
	getchar();
	return 0;
}