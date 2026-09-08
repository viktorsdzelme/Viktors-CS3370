//
#pragma once

#include <string>
#include <cstddef>
using std::string;

class CircBuf {
	const size_t CHUNK { 8 };

	char* buf_;        // heap array backing the buffer
	size_t head_;       // index where the next inserted element will go
	size_t tail_;       // index of the oldest element currently in the buffer
	size_t size_;        // number of elements currently stored
	size_t capacity_;    // number of elements buf_ can currently hold

	// Allocates a new backing array of newCapacity elements, copies the
	// current contents into it starting at index 0 (recentering the data),
	// frees the old array, and updates head_/tail_/capacity_ accordingly.
	void reallocate(size_t newCapacity);

	// Grows the backing array by exactly one CHUNK.
	void grow();

	// Grows the buffer (one CHUNK at a time) until there is room for at
	// least `additional` more elements.
	void ensureCapacity(size_t additional);

public:
	CircBuf(size_t reserve = 0);		// Number of elements you want it to be able to hold to start with.
	~CircBuf();

	CircBuf(const CircBuf& other);
	CircBuf& operator=(const CircBuf& other);
	CircBuf(CircBuf&& other) noexcept;
	CircBuf& operator=(CircBuf&& other) noexcept;

	size_t	size();
	size_t	capacity();

	void		insert(char);
	void		insert (const char*, size_t sz);
	void		insert(const string&);
	char		get();
	string	get(size_t);
	string	flush();	// Returns a string with all the characters, AND shrinks the buffer to zero.
	string	examine();
	void		shrink();	// Reduces the unused space in the buffer.
};
