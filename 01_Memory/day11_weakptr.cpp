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

struct ControlBlock{
	int strong_count; // replaces our ref_count, which is how many 
	int weak_count; // signifies how many other observers/objects are somehow using the 
		        // shared data, so we can handle errors if they try to access 
			// after strong_count = 0
};

template<typename T>
class WeakPtr; // forward declaration of our observer class
		// it will access the ptr and controlblock inside the sharedptr class

template<typename T>
class SharedPtr{
private:
        T* ptr;
        ControlBlock* cb;
        friend class WeakPtr<T>;

public:
        // Default Constructor
        SharedPtr(): ptr(nullptr), cb(nullptr){
        }

        explicit SharedPtr(T*p): ptr(p){
                cb = (ControlBlock*)MyMalloc(sizeof(ControlBlock));
                cb->strong_count = 1; // equivalent to (*cb).strong_count
                cb->weak_count = 0;
                // MODIFIED LOG: Clearer identification of container vs content
                std::cout << "[SharedPtr @ " << this << "] Constructed. Now managing Object @ " << ptr 
                          << ". Strong Ref: " << cb->strong_count << "\n";
        }
        T& operator*(){ return *ptr;}
        T* operator->(){return ptr;}
        
        // Allow copying
        SharedPtr(const SharedPtr& other){
                this->ptr = other.ptr;
                this->cb = other.cb;
                (cb->strong_count)++;
                std::cout << "[SharedPtr @ " << this << "] Copy Constructed. Managing Object @ " << ptr 
                          << ". RefCount increased to: " << cb->strong_count << "\n";
        }

        SharedPtr& operator=(const SharedPtr& other){
                if(this == &other) return *this;
                
                // Release current if exists
                if(this->cb != nullptr){
                        (this->cb->strong_count)--;
                        if(this->cb->strong_count == 0){
                                ptr->~T();
                                MyFree(this->ptr);
                                if(this->cb->strong_count == 0 && this->cb->weak_count == 0){
                                        MyFree(this->cb);
                                }
                        }
                }
                
                // Acquire new
                this->ptr = other.ptr;
                this->cb = other.cb;
                (cb->strong_count)++;
                
                // MODIFIED LOG: Explicit assignment info
                std::cout << "[SharedPtr @ " << this << "] Assigned new object. Now managing Object @ " << ptr 
                          << ". RefCount increased to: " << cb->strong_count << "\n";
                return *this;
        }

        ~SharedPtr(){
                // MODIFIED LOG: Show who is dying and what they held
                std::cout << "[SharedPtr @ " << this << "] Destructor called. held Object @ " << ptr;
                
                if (cb == nullptr) {
                    std::cout << " (Empty Ptr)\n";
                    return;
                }

                (cb->strong_count)--; // We are exiting one scope. Decrease the count.
                std::cout << ". Remaining Strong Refs: " << cb->strong_count << "\n";
                
                // Once RefCount hits 0 noone is referencing the player object so we can free memory.
                if(cb->strong_count == 0){
                        std::cout << "   -> RefCount reached 0. Deleting Object @ " << ptr << "!\n";
                        ptr->~T();
                        MyFree(ptr);
                        if(cb->strong_count == 0 && cb->weak_count == 0){
                                MyFree(cb);
                                std::cout << "   -> ControlBlock freed (0 Strong, 0 Weak)\n";
                        }
                }
        }
};

template<typename T>
class WeakPtr{
private:
        T* ptr;
        ControlBlock* cb;
public:
        explicit WeakPtr(SharedPtr<T>& other){
                this->ptr = other.ptr;
                this->cb = other.cb;
                (cb->weak_count)++;
        }

        WeakPtr(){
                ptr = nullptr;
                cb = nullptr;
        }
        ~WeakPtr(){
                if(cb == nullptr) return;
                (cb->weak_count)--;
                if(cb->strong_count == 0 && cb->weak_count ==0){
                        MyFree(cb);
                        std::cout << "   -> WeakPtr destroyed. ControlBlock freed.\n";
                }
        }

};


struct Child;
struct Parent{
        SharedPtr<Child> child;
        ~Parent(){
                std::cout << "!! [Parent Object @ " << this << "] Destructor executing !!\n";
        }
};

struct Child{
        WeakPtr<Parent> parent; // Weak Link, we directly don't access parent,
                                // but through a link
        ~Child(){
                std::cout << "!! [Child Object  @ " << this << "] Destructor executing !!\n";
        }
};

int main(){
        std::cout << "--- Cycle Test Start ---\n" << std::endl;
        {
                //Allocating raw memory
                void* p_mem = MyMalloc(sizeof(Parent));
                void* c_mem = MyMalloc(sizeof(Child));

                //initializing parent and child members to nullptr
                Parent* p_raw = new(p_mem) Parent(); // implicit default constructor compiler generates
                Child* c_raw = new(c_mem) Child();
                
                // --- PRINTING THE MAP (LEGEND) ---
                std::cout << "=== MEMORY MAP LEGEND ===" << std::endl;
                std::cout << "1. Parent Object is located at: " << p_raw << std::endl;
                std::cout << "   (Inside Parent) The 'child' member is at: " << &(p_raw->child) << std::endl;
                std::cout << "2. Child Object is located at:  " << c_raw << std::endl;
                std::cout << "=========================\n" << std::endl;

                // Passing this empty pointer
                std::cout << "--- Creating 'p' (Stack Variable) ---" << std::endl;
                SharedPtr<Parent> p(p_raw);
                
                std::cout << "--- Creating 'c' (Stack Variable) ---" << std::endl;
                SharedPtr<Child> c(c_raw);
                
                std::cout << "\nLink Parent -> Child (Strong)"<<std::endl;
                p->child = c;

                std::cout << "Link Child -> Parent (Weak)"<<std::endl;
                c->parent = WeakPtr<Parent>(p); //  constructing weakpointer<parent> using constructor
                
                std::cout << "\n--- End of Scope Imminent (Stack Unwinding) ---"<<std::endl;
        }
        std::cout << "--- Program End ---"<<std::endl;
        return 0;
}

