#include <iostream>
#include <cstdlib>

int globalVar = 10;

void myFunction(){
}

void stackGrowth(int depth){
	int localVar = 100;
	std::cout<<"localVar Address: "<<&localVar<<std::endl;
	if(depth < 3){
		stackGrowth(depth+1);
	}
}

int main(){
	int stackVar1 = 20;
	
	int* heapVar = new int(30);

	int stackVar2 = 40;

	std::cout<<"--- Memory Map ---" << std::endl;
	std::cout<<"Global Var Address:  "<<&globalVar<<std::endl;
	std::cout<<"Heap Var Address:    "<<heapVar<<std::endl;
	std::cout<<"Stack Var 1 Address: "<<&stackVar1<<std::endl;
	std::cout<<"Stack Var 2 Address: "<<&stackVar2<<std::endl;
	std::cout<<"Function Address:    "<<(void*)&myFunction<<std::endl;
	stackGrowth(1);
	delete heapVar;
	return 0;

}
