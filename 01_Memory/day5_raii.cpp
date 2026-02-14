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

template<typename T>

class ScopedPtr{
private:
	T* ptr;
public:
	// Constructor: Takes the ownership of the ptr
	
	ScopedPtr(const ScopedPtr&) = delete;
	ScopedPtr& operator = (const ScopedPtr&) = delete;
	explicit ScopedPtr(T*p): ptr(p){
		std::cout<<"ScopedPtr: Acquired ownership of "<<ptr<<std::endl;
	}
	// Destructor: Automatically called when scope ends
	~ScopedPtr(){
		std::cout<<"ScopedPtr: releasing "<<ptr<<" automatically."<<std::endl;
		MyFree(ptr);
	}

	// Operator Overloads to make it behave like real pointers
	T& operator*(){ return *ptr;} // when *player is called compiler does player.operator*() so *player get's the actual player object existing in heap ie *ptr, then we can do(*player).hp, (*player).score, basically *player = *ptr 
	T* operator->(){return ptr;} // when player->hp is called compiler does (player.operator->())->hp, so basically player->hp is ptr->hp, which means the returned thing is the actual pointer.
};

struct Player{
	int hp;
	int score;
};



int main(){
	std::cout<<"--- Entering Scope ---"<<std::endl;
	{
	ScopedPtr<Player> player((Player*)MyMalloc(sizeof(Player)));
	
	player->hp = 100;
	player->score = 5000;
	
	std::cout<<"Player HP: "<<player->hp << std::endl;
	std::cout<<"Player Address: "<<&(*player)<<std::endl;

	std::cout<<"--- Exiting Scope ---"<<std::endl;
	}
	std::cout<<"--- Back in Main ---"<<std::endl;
        return 0;
}
