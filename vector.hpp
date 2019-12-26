
#include <algorithm>
#include <cassert>
#include <memory>
#include <iostream>


template<class T,size_t BLOCK_COUNT>
class allocator {
    private:
        T  *_used_blocks;
        void  **_free_blocks;
        uint64_t _free_count;

    public:
        using value_type = T;
        using pointer = T * ;
        using const_pointer = const T*;
        using size_type = std::size_t;

        allocator() {
            static_assert(BLOCK_COUNT>0);
            _used_blocks = (pointer) malloc(sizeof(T) * BLOCK_COUNT);
            _free_blocks = (void**) malloc(sizeof (void*)*BLOCK_COUNT);

            for (uint64_t i = 0; i < BLOCK_COUNT; i++) 
                _free_blocks[i] = _used_blocks + i * sizeof(T);
            _free_count = BLOCK_COUNT;       
        }

        ~allocator(){
            delete _free_blocks;
            delete _used_blocks;
        }

        
        template<typename U>
        struct rebind {
            using other = allocator<U,BLOCK_COUNT>;
        };

        T* allocate(size_t n) {
            T *result = nullptr;
            if (_free_count > 0) {
                result = (T*)_free_blocks[--_free_count];
            } else {
                std::cout << "allocator: No memory exception" << std::endl;
            }
            return result;
        }

        void deallocate(T* pointer,  size_t ) {
            _free_blocks[_free_count++] = pointer;
        }


        template<typename U, typename ...Args>
        void construct(U *p, Args &&...args) {
            new (p) U(std::forward<Args>(args)...);
        }

        void destroy(pointer p) {
            p->~T();
        }
};

template <typename T>
class TVector { 
    public:
        using value_type = T;
        using iterator = value_type*;
        
        TVector(): 
            already_used_(0), storage_size_(0), storage_(nullptr)
        {
        }

        TVector(int size, const value_type& default_value = value_type()):
            TVector()
        {
            assert(size >= 0);

            if (size == 0) {
                return;
            }

            already_used_ = size;
            storage_size_ = size;
            storage_ = std::make_unique<value_type[]>(size);

            std::fill(storage_.get(), storage_.get() + already_used_, default_value);
        }

        int size() const
        {
            return already_used_;
        }

        bool empty() const
        {
            return size() == 0;
        }

        iterator begin() const
        {
            return storage_.get();
        }
        
        iterator end() const
        {
            if (storage_.get()) {
                return storage_.get() + already_used_;
            }

            return nullptr;
        }
        void insert(iterator pos, value_type val) {
            if (already_used_ < storage_size_) {
                std::copy(pos, storage_.get() + already_used_, pos + 1);
                *pos = val;
                ++already_used_;
                return;
            }
            int next_size = 1;
            if (storage_size_) {
                next_size = storage_size_ * 2;
            }
            TVector next(next_size);
            next.already_used_ = already_used_;

            if (storage_.get()) {
                std::copy(storage_.get(), storage_.get() + storage_size_, next.storage_.get());
            }
            next.insert(pos, val);
            Swap(*this, next);
        }
        
        void erase(iterator pos) {
            std::copy(pos + 1, storage_.get() + already_used_, pos);
            --already_used_;
        }
        
        friend void Swap(TVector& lhs, TVector& rhs)
        {
            using std::swap;

            swap(lhs.already_used_, rhs.already_used_);
            swap(lhs.storage_size_, rhs.storage_size_);
            swap(lhs.storage_, rhs.storage_);
        }

        TVector& operator=(TVector other)
        {
            Swap(*this, other);
            return *this;
        }

        TVector(const TVector& other):
            TVector()
        {
            TVector next(other.storage_size_);
            next.already_used_ = other.already_used_;

            if (*(other.storage_) ) {
                std::copy(other.storage_.get(), other.storage_.get() + other.storage_size_,
                        next.storage_.get());
            }

            swap(*this, next);
        }

        ~TVector()
        {
            storage_size_ = 0;
            already_used_ = 0;
        }

        void push_back(const value_type& value)
        {
            if (already_used_ < storage_size_) {
                storage_[already_used_] = value;
                ++already_used_;
                return;
            }
            int next_size = 1;
            if (storage_size_) {
                next_size = storage_size_ * 2;
            }
            TVector next(next_size);
            next.already_used_ = already_used_;

            if (storage_.get()) {
                std::copy(storage_.get(), storage_.get() + storage_size_, next.storage_.get());
            }
            next.push_back(value);
            Swap(*this, next);
        }
        
        value_type& At(int index)
        {
            if (index < 0 || index > already_used_) {
                throw std::out_of_range("You are doing this wrong!");
            }

            return storage_[index];
        }

        value_type& operator[](int index)
        {
            return At(index);
        }

    private:
        int already_used_;
        int storage_size_;
        std::unique_ptr<value_type[]> storage_;
};
