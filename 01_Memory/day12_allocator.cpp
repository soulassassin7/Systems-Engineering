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

template<typename T>
struct MyAllocator{ // we can even declare it as class in cpp class and struct 
		    // are 99 percent same excpet for the fact that by default 
		    // members are private in class and public in struct by default
	T* allocate(size_t n){
		T* ptr = (T*)MyMalloc(n*sizeof(T));
		return ptr;
	}
	
	void deallocate(T* p){
		if(p == nullptr) return;
		MyFree(p);
	}
	
	template<typename... Args>
	void construct(T*p, Args&&... args){
		new (p) T(std::forward<Args>(args)...); // This is called perfect forwarding
	// I accept ANY number of arguments, of ANY type,
	// and I will pass them to the constructor exactly as I received them
	}

	/*
	How it expands: If you call allocator.construct(ptr, 10, "Hello"):

	Args becomes [int, const char*].

	args becomes [10, "Hello"].

	The line becomes: new (ptr) T(std::forward<int>(10), std::forward<const char*>("Hello"));
	*/

	void destroy(T*p){
		p->~T();
	}
};

	

template <typename T, typename Alloc = MyAllocator<T>>
class MyVector{
private:
        size_t size;
        size_t capacity;
        T* data;
	Alloc allocator;
        void resize(){
                size_t new_capacity = capacity*2;
                T* new_data = allocator.allocate(new_capacity);
                for(size_t i = 0; i<size; ++i){
			allocator.construct(&new_data[i],std::move(data[i]));
			allocator.destroy(&data[i]);
                }
                allocator.deallocate(data);
                data = new_data;
                capacity = new_capacity;
        }

public:
        MyVector(){
                size = 0;
                capacity = 2;
                data = allocator.allocate(capacity);
        }

        ~MyVector(){
                for(int i = 0; i<size; ++i){
			allocator.destroy(&data[i]);
                }
                allocator.deallocate(data);
        }
	// copy constructor
	MyVector (const MyVector& other){
		this->size = other.size;
		this->capacity = other.capacity;
		this->data = allocator.allocate(this->capacity);
		for(size_t i = 0; i<this->size; ++i){
			(this->allocator).construct(&(this->data)[i],other.data[i]);
		}
	} // shallow copy is not allowed when we have destructor

	
	// Handling move
	MyVector(MyVector&& other) noexcept{
		this->size = other.size;
		this->capacity = other.capacity;
		this->data = other.data;
		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;
	}
	
	// deep copy both above and below is the assignment operator
	// operator overloading
	MyVector& operator = (const MyVector& other){
		if(this == &other) return *this;
		
		T* new_address = (this->allocator).allocate(other.capacity);
		for(size_t i = 0; i<other.size; ++i){
			(this->allocator).construct(&new_address[i], other.data[i]);
		}

		for(size_t i = 0; i<(this->size); ++i){
			(this->allocator).destroy(&(this->data)[i]);
		}

		
		(this->allocator).deallocate(this->data);
		this->size = other.size;
		this->capacity = other.capacity;
		this->data = new_address;
		return *this;
	}

	// move operator
	MyVector& operator =(MyVector&& other){
		if(this == &other) return *this;
		for(size_t i = 0; i<(this->size); ++i){
			(this->allocator).destroy(&(this->data)[i]);
		}
		(this->allocator).deallocate(this->data);
		this->size = other.size;
		this->capacity = other.capacity;
		this->data = other.data;
		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;
		return *this;
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
	// for r-value(temporary values)
        void push_back(T&& value){
                if(size == capacity){
                        std::cout<<"Old Capacity: "<<capacity<<std::endl;
                        resize();
                        std::cout<<"Resized the vector successfully, new capacity: "<<capacity<<std::endl;
                }
		allocator.construct(&data[size],std::move(value));
                ++size;

        }
	// for l-values both mutable and immutable(const) types, copying the value
	void push_back(const T& value){
		if(size == capacity){
			std::cout<<"Old capacity: "<<capacity<<std::endl;
			resize();
			std::cout<<"Resized the vector successfully, new capacity: "<<capacity<<std::endl;
		}
		allocator.construct(&data[size], value);
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
    std::cout << "============= TEST 1: Push & Resize =============" << std::endl;
    MyVector<TestObj> v1;
    {
        std::cout << "[Step 1] Pushing r-value (10)..." << std::endl;
        v1.push_back(TestObj(10)); // Should Move

        std::cout << "\n[Step 2] Pushing l-value (20)..." << std::endl;
        TestObj t20(20);
        v1.push_back(t20); // Should Copy

        std::cout << "\n[Step 3] Pushing (30) to trigger resize..." << std::endl;
        v1.push_back(TestObj(30)); 
        // Expected: 
        // 1. Move construct 30
        // 2. Resize moves 10 and 20 to new buffer
        // 3. Destroys old 10 and 20
    }

    std::cout << "\n============= TEST 2: Copy Semantics =============" << std::endl;
    {
        std::cout << "[Step 1] Copy Constructor (v2 from v1)..." << std::endl;
        MyVector<TestObj> v2 = v1; // Deep Copy
        
        std::cout << "\n[Step 2] Copy Assignment (v3 = v1)..." << std::endl;
        MyVector<TestObj> v3;
        v3.push_back(TestObj(999)); // Garbage data to ensure it gets cleaned up
        v3 = v1; 
        // Expected: Destroys 999, then Deep Copies v1
    } // v2 and v3 die here. v1 stays alive.

    std::cout << "\n============= TEST 3: Move Semantics =============" << std::endl;
    {
        std::cout << "[Step 1] Move Constructor (v4 from moved v1)..." << std::endl;
        MyVector<TestObj> v4 = std::move(v1); 
        // Expected: Steals pointers. v1 is now empty. No "Copied" logs.

        std::cout << "\n[Step 2] Move Assignment (v5 = moved v4)..." << std::endl;
        MyVector<TestObj> v5;
        v5.push_back(TestObj(888)); // Existing data
        v5 = std::move(v4);
        // Expected: Destroys 888, then steals v4.
    } 

    std::cout << "\n============= TEST 4: Edge Cases =============" << std::endl;
    {
        MyVector<TestObj> v6;
        v6.push_back(TestObj(5));
        
        std::cout << "[Step 1] Self Assignment Check..." << std::endl;
        v6 = v6; // Should do nothing (no crash, no destroy)
        
        std::cout << "[Step 2] Self Move Check..." << std::endl;
        v6 = std::move(v6); // Should do nothing
    }

    std::cout << "\n============= End of Main =============" << std::endl;
    return 0;
}
