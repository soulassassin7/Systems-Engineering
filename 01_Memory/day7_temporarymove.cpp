#include<iostream>
#include<unistd.h>
#include<algorithm>
#include<cstring>
#include<utility>

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
        std::memcpy(new_ptr,ptr,std::min(block->size,size)); //std::min is defined in algorithm header that's why we did #inlcude<algorithm>
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

template<typename T>
class SharedPtr{
private:
        T* ptr;
        int* ref_count;
public:
        explicit SharedPtr(T*p): ptr(p){
                ref_count = (int*)MyMalloc(sizeof(int));
                *ref_count = 1;
                std::cout<<"SharedPtr Created. RefCount: "<<*ref_count<<std::endl;
        }
	
	int use_count(){
		if(ref_count == nullptr) return 0;
		return *ref_count;
	}

        T& operator*(){ return *ptr;}
        T* operator->(){return ptr;}
        // Allow copying
        SharedPtr(const SharedPtr& other){
                this->ptr = other.ptr;
                this->ref_count = other.ref_count;
                (*ref_count)++;
                std::cout<<"Shared Ptr Copied. RefCount is now: "<<*ref_count<<std::endl;
        }

        SharedPtr& operator = (const SharedPtr& other){
                // check if we are actually doing p1 = p1 assignment
                if(this == &other){
                        return *this;
                }

                // clean up old data, p2 will point to p1 so the current reference count must decrease because we will no longer be referencing the old data
                (*ref_count)--;
                if(*ref_count == 0){
                        std::cout<<"Assignment: Old data refrence dropped to 0. Freeing the memory!"<<std::endl;
                        MyFree(ptr);
                        MyFree(ref_count);
                }

                this->ptr = other.ptr;
                this->ref_count = other.ref_count;

                (*ref_count)++;

                std::cout<<"Assignment: Switched to new data. New RefCount: "<<*ref_count<<std::endl;
                return *this;
        }
        // && signifies that we are  taking temporary object other
        SharedPtr(SharedPtr&& other) noexcept{
                // Steal the data
                this->ptr = other.ptr;
                this->ref_count = other.ref_count;

                // nullify the other so when it dies it does not destroy the data
                other.ptr = nullptr;
                other.ref_count = nullptr;
                std::cout<<"SharedPtr Moved(Stolen!). RefCount remains: "<<*ref_count<<std::endl;
        }
        ~SharedPtr(){
                if(ptr == nullptr) return;
                (*ref_count)--; // We are exiting one scope. Decrease the count.
                std::cout<<"SharedPtr destroyed. refCount is now: "<<*ref_count<<std::endl;
                // Once RefCount hits 0 noone is referencing the player object so we can free memory.
                if(*ref_count == 0){
                        std::cout<<"RefCount reached 0. Freeing memory!"<<std::endl;
                        MyFree(ptr);
                        MyFree(ref_count);
                }
        }
};

struct Player{
        int hp;
        int score;
};


SharedPtr<Player> createPlayer(){
        SharedPtr<Player> temp((Player*)MyMalloc(sizeof(Player)));
        temp->hp = 100;
        return temp; // returning a temporary so  MOVE will be triggered
}

int main(){
        std::cout<<"--- Main Start ---"<<std::endl;
 	SharedPtr<Player> p1((Player*)MyMalloc(sizeof(Player)));
	p1->hp = 100;
	std::cout<<"p1 RefCount: "<<p1.use_count()<<std::endl;
	std::cout<<"--- Move ---" <<std::endl;
	// Forcing a Move from p1 to p2 using std::move()
        // SharedPtr<Player> p2 = createPlayer();
	SharedPtr<Player> p2 = std::move(p1);
	std::cout<<"p2 RefCount: "<<p2.use_count()<<std::endl;
	std::cout<<"p1 use_count: "<<p1.use_count()<<std::endl;

        std::cout<<"P2 HP: "<<p2->hp<<std::endl;
	
	std::cout<<"--- Main End ---"<<std::endl;
        return 0;
}
