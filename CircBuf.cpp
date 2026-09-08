#include "CircBuf.h"

#include <algorithm>

CircBuf::CircBuf(size_t reserve)
    : buf_(nullptr), head_(0), tail_(0), size_(0), capacity_(0)
{
    ensureCapacity(reserve);
}

CircBuf::~CircBuf()
{
    delete[] buf_;
}

CircBuf::CircBuf(const CircBuf& other)
    : buf_(nullptr), head_(other.head_), tail_(other.tail_),
      size_(other.size_), capacity_(other.capacity_)
{
    if (capacity_ > 0) {
        buf_ = new char[capacity_];
        std::copy(other.buf_, other.buf_ + capacity_, buf_);
    }
}

CircBuf& CircBuf::operator=(const CircBuf& other)
{
    if (this != &other) {
        char* newBuf = nullptr;
        if (other.capacity_ > 0) {
            newBuf = new char[other.capacity_];
            std::copy(other.buf_, other.buf_ + other.capacity_, newBuf);
        }

        delete[] buf_;
        buf_ = newBuf;
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        capacity_ = other.capacity_;
    }
    return *this;
}

CircBuf::CircBuf(CircBuf&& other) noexcept
    : buf_(other.buf_), head_(other.head_), tail_(other.tail_),
      size_(other.size_), capacity_(other.capacity_)
{
    other.buf_ = nullptr;
    other.head_ = 0;
    other.tail_ = 0;
    other.size_ = 0;
    other.capacity_ = 0;
}

CircBuf& CircBuf::operator=(CircBuf&& other) noexcept
{
    if (this != &other) {
        delete[] buf_;

        buf_ = other.buf_;
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        capacity_ = other.capacity_;

        other.buf_ = nullptr;
        other.head_ = 0;
        other.tail_ = 0;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    return *this;
}

// Reallocates the backing array to hold exactly newCapacity elements,
// copying the existing elements so they start at index 0 in the new array
// (even if they wrapped around the end of the old one).
void CircBuf::reallocate(size_t newCapacity)
{
    char* newBuf = (newCapacity > 0) ? new char[newCapacity] : nullptr;

    if (size_ > 0) {
        size_t firstPartLen = std::min(size_, capacity_ - tail_);
        std::copy(buf_ + tail_, buf_ + tail_ + firstPartLen, newBuf);

        size_t secondPartLen = size_ - firstPartLen;
        if (secondPartLen > 0) {
            std::copy(buf_, buf_ + secondPartLen, newBuf + firstPartLen);
        }
    }

    delete[] buf_;
    buf_ = newBuf;
    capacity_ = newCapacity;
    tail_ = 0;
    head_ = (capacity_ == 0) ? 0 : (size_ % capacity_);
}

void CircBuf::grow()
{
    reallocate(capacity_ + CHUNK);
}

void CircBuf::ensureCapacity(size_t additional)
{
    while (capacity_ - size_ < additional) {
        grow();
    }
}

size_t CircBuf::size()
{
    return size_;
}

size_t CircBuf::capacity()
{
    return capacity_;
}

void CircBuf::insert(char c)
{
    ensureCapacity(1);

    buf_[head_] = c;
    head_ = (head_ + 1) % capacity_;
    ++size_;
}

void CircBuf::insert(const char* data, size_t sz)
{
    if (sz == 0) {
        return;
    }

    ensureCapacity(sz);

    size_t firstPartLen = std::min(sz, capacity_ - head_);
    std::copy(data, data + firstPartLen, buf_ + head_);

    size_t secondPartLen = sz - firstPartLen;
    if (secondPartLen > 0) {
        std::copy(data + firstPartLen, data + sz, buf_);
    }

    head_ = (head_ + sz) % capacity_;
    size_ += sz;
}

void CircBuf::insert(const string& s)
{
    insert(s.data(), s.size());
}

char CircBuf::get()
{
    if (size_ == 0) {
        return '\0';
    }

    char c = buf_[tail_];
    tail_ = (tail_ + 1) % capacity_;
    --size_;
    return c;
}

string CircBuf::get(size_t n)
{
    n = std::min(n, size_);

    string result;
    result.reserve(n);

    size_t firstPartLen = std::min(n, capacity_ - tail_);
    result.append(buf_ + tail_, firstPartLen);

    size_t secondPartLen = n - firstPartLen;
    if (secondPartLen > 0) {
        result.append(buf_, secondPartLen);
    }

    tail_ = (capacity_ == 0) ? 0 : (tail_ + n) % capacity_;
    size_ -= n;
    return result;
}

string CircBuf::flush()
{
    string result = get(size_);
    reallocate(0); // shrink the buffer all the way down to zero
    return result;
}

string CircBuf::examine()
{
    string result(capacity_, '-');

    size_t idx = tail_;
    for (size_t i = 0; i < size_; ++i) {
        result[idx] = buf_[idx];
        idx = (idx + 1) % capacity_;
    }

    return "[" + result + "]";
}

void CircBuf::shrink()
{
    size_t newCapacity = capacity_;
    while (newCapacity >= CHUNK && newCapacity - CHUNK >= size_) {
        newCapacity -= CHUNK;
    }

    if (newCapacity != capacity_) {
        reallocate(newCapacity);
    }
}
