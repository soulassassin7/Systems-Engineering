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


int main(){
 	std::cout<<"--- Coalescing Test ---"<<std::endl;
	void* p1 = MyMalloc(100);
	void* p2 = MyMalloc(100);

	std::cout<<"P1: "<<p1<<std::endl;
	std::cout<<"P2: "<<p2<<std::endl;
	
	MyFree(p1);
	MyFree(p2);

	void* p3 = MyMalloc(150);
	std::cout<<"P3: "<<p3<<std::endl;
	
	if(p3 == p1) std::cout<<"SUCCESS: Blocks merged! P3 took P1's spot."<<std::endl;
	else std::cout<<"FAILURE: Fragmentation detected. P3 moved to new memory address."<<std::endl;
        return 0;
}
