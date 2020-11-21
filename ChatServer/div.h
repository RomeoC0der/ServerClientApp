#pragma once
namespace KGB {
	template <typename T>
	class div
	{
	public:
		div() {
			this->arr = new T[_capacity_];
		};
		~div() {};
		void reserve(int length);
		T* begin() { return &this->arr[0]; };
		T* end() { return &this->arr[this->_length_]; };
		void reserve(size_t size);
		void clear();
		void erase(int start, int end);
		void erase(int index);
		void emplace_back(T element);
		void insert_front(T element);
		void pop_back();
		void pop_front();
		int length();
		int capacity();
		T& at(int index);
	private:
		int _capacity_{ 1 };//all memory
		int _length_{ 0 };//current length
		size_t _size_{ sizeof(T) };
		T* arr;
		void recreate_arr();
		void copy_arr(int start, int end, T* arr, T* output);
	};

	template<typename T>
	inline void div<T>::reserve(int length)
	{
		if (this->_capacity_ > 0)
		{
			T* buffer = new T[this->_length_];
			copy_arr(0, this->_length_ - 1, this->arr, buffer);
			this->arr = new T[length];
			copy_arr(0, this->_length_ - 1, buffer, this->arr);
			delete[] buffer;
			buffer = nullptr;
		}
		else {
			this->arr = new T[length];
		}
		this->_capacity_ = length;

	}

	template<typename T>
	inline void div<T>::reserve(size_t size)
	{
		this->_capacity_ = size / sizeof(T);
	}

	template<typename T>
	inline void div<T>::clear()
	{
		this->_length_ = 0;
		memcpy(this->begin(), &this->_length_, sizeof(T) * this->_capacity_);//_length_ == 0
	}

	template<typename T>
	inline void div<T>::erase(int start, int end)
	{
	}

	template<typename T>
	inline void div<T>::erase(int index)
	{
		/*T* first_array_part = new T[this->_length_ - index];
		T* second_array_part = new T[this->_length_ - index];
		copy_arr(0, index - 1, this->arr, first_array_part);
		copy_arr(index + 1, this->_length_, this->arr, second_array_part);
		--this->_length_;
		copy_arr(0, index -1, first_array_part, )*/
		for (int i = 0; i < this->_length_; i++)
		{
			if (i > index)
			{
				this->arr[i - 1] = arr[i];
			}
		}
		--this->_length_;
	}
	template<typename T>
	inline void div<T>::emplace_back(T element)
	{
		if (this->_capacity_ > this->_length_)
		{
			arr[this->_length_] = element;
			this->_length_++;
		}
		else {
			reserve(++this->_capacity_);
			emplace_back(element);
		}
	}

	template<typename T>
	inline void div<T>::insert_front(T element)
	{

		if (this->_capacity_ > this->_length_)
		{
			T* buffer = new T[this->_capacity_];
			copy_arr(0, this->_size_, this->arr, buffer);
			for (int i = 1; i < this->_size_; i++)
			{
				this->arr[i] = buffer[i - 1];
			}
			arr[0] = element;
			this->_length_++;
			delete[] buffer;
			buffer = nullptr;
		}
		else {
			reserve(++this->_capacity_);
			insert_front(element);
		}
	}

	template<typename T>
	inline void div<T>::pop_back()
	{
		this->reserve(--this->_length_);
	}

	template<typename T>
	inline void div<T>::pop_front()
	{
		T* arr_ = new T[--_length_];
		copy_arr(1, _length_, this->arr, arr_);
		reserve(_length_);
		copy_arr(0, _length_, arr_, this->arr);
		delete[] arr_;
		arr_ = nullptr;
	}

	template<typename T>
	inline int div<T>::length()
	{
		return this->_length_;
	}

	template<typename T>
	inline int div<T>::capacity()
	{
		return this->_capacity_;
	}

	template<typename T>
	inline T& div<T>::at(int index)
	{
		/*skip all shit*/
		return arr[index];
	}

	template<typename T>
	inline void div<T>::copy_arr(int start, int end, T* arr, T* output)
	{
		int new_length = end - start;
		int n_counter = 0;
		for (int i = start; i <= end; i++)
		{
			output[n_counter] = arr[i];
			__asm {
				mov ebx, 0;
				mov ebx, n_counter;
				add ebx, 1;
				mov n_counter, ebx;
				mov ebx, 0;
			}
		}
	}
}