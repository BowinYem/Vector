#ifndef VEC
#define VEC
#include <string>
#include <memory>
#include <utility>
#include <algorithm>
#include <initializer_list>

template<typename T> class Vec{
	
	//***FRIEND DECLARATIONS***
	friend bool operator== (const Vec<T>& lhs, const Vec<T>& rhs)
	{
		return (lhs.array_size() == rhs.array_size()) && 
			std::equal(lhs.element, lhs.first_free, rhs.element);
	}

	friend bool operator!= (const Vec<T>& lhs, const Vec<T>& rhs) { return !(lhs == rhs); }

	friend bool operator< (const Vec<T>& lhs, const Vec<T>& rhs)
	{
		return std::lexicographical_compare(lhs.element, lhs.first_free, rhs.element, rhs.first_free);
	}

	friend bool operator> (const Vec<T>& lhs, const Vec<T>& rhs) { return rhs < lhs; }
	friend bool operator<= (const Vec<T>& lhs, const Vec<T>& rhs) { return !(rhs < lhs); }
	friend bool operator>= (const Vec<T>& lhs, const Vec<T>& rhs) { return !(lhs < rhs); }

	public:
	
	//***COPY CONTROL OPERATIONS/CONSTRUCTORS/OPERATORS***

	Vec() : element(NULL), first_free(NULL), cap(NULL){}

	Vec(const Vec& arg)
	{
		auto data = alloc_n_copy(arg.begin(), arg.end());
		element = data.first;
		first_free = cap = data.second;
	}

	Vec(Vec&& arg) noexcept : element(arg.element), first_free(arg.first_free), cap(arg.cap)
	{
		arg.element = nullptr;
		arg.first_free = nullptr;
		arg.cap = nullptr;
	}

	Vec(std::initializer_list<T> arg_list)
	{
		auto data = alloc_n_copy(arg_list.begin(), arg_list.end());
		element = data.first;
		first_free = cap = data.second;
	}

	~Vec() { free(); }

	Vec& operator= (const Vec& rhs)
	{
		auto data = alloc_n_copy(rhs.begin(), rhs.end());
		free();
		element = data.first;
		first_free = cap = data.second;
		return *this;
	}

	Vec& operator= (Vec&& rhs) noexcept
	{
		if (this != &rhs)
		{
			free();
			element = rhs.element;
			first_free = rhs.first_free;
			cap = rhs.cap;
			rhs.element = rhs.first_free = rhs.cap = nullptr;
		}
		return *this;
	}
	
	
	T& operator[] (std::size_t n) { return element[n]; }
	const T& operator[] (std::size_t n) const { return element[n]; }
	
	//***PUBLIC FUNCTIONS***

	size_t max_capacity() const {return cap - element;}
	size_t array_size() const {return first_free - element;}
	T* begin() const{return element;}
	T* end() const {return first_free;}
	
	//Declare a function that pushes back a string
	void push_back(const T& n)
	{
		chk_n_alloc();
		alloc.construct(first_free++, n);
	}
	
	//Capacity returns the number of objects you can construct before reallocating
	size_t capacity() const {return cap - first_free;}

	void reserve(size_t n)
	{
		if (n > max_capacity())
		{
			auto newData = alloc.allocate(n);

			auto newDest = newData;
			auto oldSource = element;

			for (size_t i = 0; i != array_size(); ++i)
				alloc.construct(newDest++, std::move(*oldSource++));

			free();
			element = newData;
			first_free = newDest;
			cap = element + n;
		}
	}

	void resize(size_t n, const T& init)
	{
		auto size = array_size();
		if (n > size)
			for (size_t i = 0; i != n - size; ++i)
				push_back(init);
		else if (n < size)
			for (size_t i = 0; i != size - n; ++i)
				alloc.destroy(--first_free);
	}

	void resize(size_t n)
	{
		auto size = array_size();
		if (n > size)
			for (size_t i = 0; i != n - size; ++i)
			{
				chk_n_alloc();
				alloc.construct(first_free++);
			}
		else if (n < size)
			for (size_t i = 0; i != size - n; ++i)
				alloc.destroy(--first_free);
	}
	
	template<typename... Args> void emplace_back(Args&&... args)
	{
		chk_n_alloc();
		alloc.construct(first_free++, std::forward<Args>(args)...);
	}
	
	private:
	//***PRIVATE FUNCTIONS***
	void chk_n_alloc() { if (max_capacity() == array_size()) reallocate();}

	std::pair<T*, T*>  alloc_n_copy(const T* b, const T* e)
	{
		auto data = alloc.allocate(e - b);
		return { data, std::uninitialized_copy(b, e, data) };
	}

	void free()
	{
		if (element)
			std::for_each(element, first_free, [this](T& p)
				{
					alloc.destroy(&p);
				});
		alloc.deallocate(element, cap - element);
	}

	void reallocate()
	{
		auto newSize = array_size() ? 2 * array_size() : 1;
		auto newData = alloc.allocate(newSize);

		auto newDest = newData;
		auto source = element;

		for (size_t i = 0; i != array_size(); ++i)
			alloc.construct(newDest++, std::move(*source++));

		free();
		element = newData;
		first_free = newDest;
		cap = element + newSize;
	}
	
	//***PRIVATE DATA MEMBERS***
	std::allocator<T> alloc;
	T* element;
	T* first_free;
	T* cap;
};

#endif