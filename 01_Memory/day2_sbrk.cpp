#include<iostream>
#include<unistd.h>

using namespace std;

int main(){
	long long old = (long long)sbrk(0);
	cout<<sbrk(0)<<endl;
	sbrk(4096);
	cout<<sbrk(0)<<endl;
	long long cur = (long long)sbrk(0);
	cout<<"The difference is: "<<cur - old<<" bytes"<<endl;

}
