#include<iostream>

using namespace std;

int main(){
	struct Block{
		size_t size;
		bool is_free;
		Block* next;
	};

	cout<<sizeof(Block)<<endl;
	return 0;
}
