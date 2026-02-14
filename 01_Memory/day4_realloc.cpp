#include<iostream>
#include<unistd.h>
#include<algorithm>
#include<cstring>

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



void coalesce(){
        Block* cur = head;


        while(cur && cur->next){
                if(cur->is_free && cur->next->is_free){
                        cur->size += sizeof(Block)+cur->next->size;
                        cur->next = cur->next->next;
                } else{
                        cur = cur->next;
                }
        }
}

void MyFree(void* ptr){
        if(!ptr) return;

        Block* block = (Block*)ptr -1;
        block->is_free = true;
        coalesce();
}


void* MyRealloc(void* ptr, size_t size){
	if(!ptr) return MyMalloc(size);
	if(size == 0){
		MyFree(ptr);
		return nullptr;
	}

	Block* block = (Block*)ptr-1;

	if(block->size >= size) return ptr;

	void* new_ptr = MyMalloc(size);
	std::memcpy(new_ptr,ptr,std::min(block->size,size));
	MyFree(ptr);
	return new_ptr;
}

int main(){
	std::cout<<"--- Realloc Test ---"<< std::endl;
	int * arr = (int*)MyMalloc(2*sizeof(int));
	arr[0] = 10;
	arr[1] = 20;
	std::cout<<"Original Address: "<<arr<<std::endl;
	std::cout<<"Data: "<<arr[0]<<", "<<arr[1]<<std::endl;
	
	int* new_arr = (int*)MyRealloc(arr,5*sizeof(int));
	std::cout<<"New Address:      "<<new_arr<<std::endl;
	std::cout<<"Data check: "<<new_arr[0]<<", "<<new_arr[1]<<std::endl;
	new_arr[2] = 30;
	new_arr[3] = 40;
	new_arr[4] = 50;
	std::cout<<"New Data: "<<new_arr[2]<<" "<<new_arr[3]<<" "<<new_arr[4]<<std::endl;
	MyFree(new_arr);
        return 0;
}
