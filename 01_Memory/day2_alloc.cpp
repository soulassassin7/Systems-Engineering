#include<iostream>
#include<unistd.h>

struct Block{
	size_t size;
	bool is_free;
	Block* next;
};

Block* head = nullptr;

Block* request_space(Block* last, size_t size){
	Block* block = (Block*)sbrk(0);
	
	void* request = sbrk(size + sizeof(Block));
	
	if(request == (void*) -1){
		return  nullptr;
	}

	if(last){
		last->next = block;
	}

	block->size = size;
	block->is_free = false;
	block->next = nullptr;
	return block;
}

Block* find_free_block(Block** last, size_t size){
	Block* current = head;
	while(current != NULL){
		if(current->is_free && current->size>=size) return current;
		*last = current;
		current = current->next;
	}
	return nullptr;
}

void* MyMalloc(size_t size){
	Block* block;
	if(size<=0) return nullptr;
	
	if(head == NULL){
		block = request_space(nullptr,size);
		if(block == nullptr) return nullptr;
		head = block;
		
	} else{
		Block* last = head;
		block = find_free_block(&last,size);
		if(block == nullptr){
			block = request_space(last,size);
			if(block == nullptr) return  nullptr;
		}
		else{
			block->is_free = false;
		}
	}
	return (void*)(block+1);
}
int main(){
	std::cout<< "1. Allocating 100 bytes (int array)..." << std::endl;
	int* arr = (int*)MyMalloc(100*sizeof(int));
	arr[0] = 42;
	std::cout<< " Address: "<< arr << ", Value: " << arr[0] << std::endl;
	
	std::cout << "2. Allocating 1 char..." << std:: endl;
	char* c = (char*) MyMalloc(1);
	*c = 'A';
	std::cout << "Address: "<< (void*)c << ", Value: " << *c << std:: endl;
	long diff = (long)c - (long)arr;
	std::cout << " Difference: "<< diff << " bytes" << std::endl;
	return 0;
}

