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
        std::memcpy(new_ptr,ptr,std::min(block->size,size));
        MyFree(ptr);
        return new_ptr;
}

template <typename T>
class MyVector{
private:
        size_t size;
        size_t capacity;
        T* data;
        void resize(){
                size_t new_capacity = capacity*2;
                T* new_data = (T*)MyMalloc(new_capacity*sizeof(T));
                for(size_t i = 0; i<size; ++i){
                        new (&new_data[i]) T(std::move(data[i]));
			data[i].~T();
                }
                MyFree(data);
                data = new_data;
                capacity = new_capacity;
        }

public:
        MyVector(){
                size = 0;
                capacity = 2;
                data = (T*)MyMalloc(capacity*sizeof(T));
        }

        ~MyVector(){
		for(int i = 0; i<size; ++i){
			data[i].~T(); // calling destructor on each individual block, this frees
		}
                MyFree(data);
        }
        // operator overloading, this helps define what [] means to compiler when applied with
        // a variable/object of MyVector data type. So when v[index] happens it gets the value
        // data[index]

        T& operator[](size_t index){
                return data[index];
        }

        T* begin(){
                return data;
        }

        T* end(){
                return data+size;
        }

        void push_back(T value){
                if(size == capacity){
                        std::cout<<"Old Capacity: "<<capacity<<std::endl;
                        resize();
                        std::cout<<"Resized the vector successfully, new capacity: "<<capacity<<std::endl;
                }
		new (&data[size]) T(std::move(value)); // new syntax of Placement New, new (pointer_to_memory) Type(value)
                // it's like new (Where?) What(value?) to assign value to 
		++size;

        }
	size_t get_size(){
		return size;
	}
};




struct TestObj{
        int id;
	TestObj(int i): id(i){
		std::cout<<"Constructed "<<id<<std::endl;
	}
	
	TestObj(const TestObj& other): id(other.id){
		std::cout<<"Copied "<<id<<std::endl;
	}
	
	TestObj(TestObj&& other) noexcept: id(other.id){
		std::cout<<"Moved "<<id<<std::endl;
		other.id = -1; //Mark as empty
	}
	~TestObj(){
		std::cout<<"Destroyed "<<id<<std::endl;
	}

};



int main(){
	std::cout<<"--- Placement New Test ---"<<std::endl;
	{
		MyVector<TestObj> v;
		std::cout<<"1. Pushing 10..."<<std::endl;
		v.push_back(TestObj(10));

		std::cout<<"\n2. Pushing 20..."<<std::endl;
		v.push_back(TestObj(20));

		std::cout<<"\n3. Pushing 30 (Trigger Resize)..."<<std::endl;
		v.push_back(TestObj(30));
		
		std::cout<<"\n4. Exiting Scope..."<<std::endl;
	}
	std::cout<<"--- End ---"<<std::endl;
        return 0;
}
