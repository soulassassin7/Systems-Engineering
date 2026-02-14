#include<iostream>

using namespace std;

int main(){
	unsigned long long data[2] = {0,0};
	int *p = (int*)&data;
	char *c = (char*)&data;
	cout<<"p Address: "<<p<<" p+1 Address: "<<(p+1)<<endl;
	cout<<"c Address: "<<(void*)c<<" c+1 Address: "<<(void*)(c+1)<<endl;
	// there are 16 bytes, first 8 for data[0] and next 8 for data[1]
	// when we do c[1], we are actually accessing the 2nd byte of data[0]
	// ie [00] [ff] [00] [00] [00] [00] [00] [00]
	// this is read when we try to print data[0] the bytes are considered in reverse order
	// this is small endianness, we get ff*16 + 00*1 = 
	c[1] = 0xff;
	cout<<hex<<data[0]<<endl;
	cout<<data[0]<<endl;
	return 0;
}
