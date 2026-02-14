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
			new_data[i] = std::move(data[i]);
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
		MyFree(data);
	}
	// operator overloading, this helps define what [] means to compiler when applied with 
	// a variable/object of MyVector data type. So when v[index] happens it gets the value 
	// data[index]

	T& operator[](size_t index){
		return data[index];
	}

	void push_back(T value){
		if(size == capacity){
			std::cout<<"Old Capacity: "<<capacity<<std::endl;
			resize();
			std::cout<<"Resized the vector successfully, new capacity: "<<capacity<<std::endl;
		} 
		data[size] = value;
		++size;
		
	}
};


struct Player{
        int hp;
        int score;
};



int main(){
	std::cout<<"--- Vector Test Start ---"<<std::endl;
	MyVector<int> v;
	v.push_back(10);
	v.push_back(20);
	
	std::cout<<"Pushing 3rd item..." <<std::endl;
	v.push_back(30);

	std::cout<<"Pushing 4th, 5th, 6th.."<<std::endl;
	v.push_back(40);
	v.push_back(50);
	v.push_back(60);

	std::cout<<"\n--- Final Results ---"<<std::endl;
	for(size_t i = 0; i<6; ++i){
		std::cout<<"v["<<i<<"] = "<<v[i]<<std::endl;
	}

        return 0;
}
