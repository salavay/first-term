#include <iostream>

#ifndef VECTOR_H
#define VECTOR_H

template<typename T>
class vector {

 public:
  using iterator = T *;
  using const_iterator = const T *;

  vector() {
    data_ = nullptr;
    capacity_ = 0;
    size_ = 0;
  }

  vector(vector const &other) { // O(N) strong
    T *new_data = nullptr;
    if (other.data_ && other.size() != 0) {
      new_data = static_cast<T *>(operator new(other.size() * sizeof(T)));
    }
    size_t i = 0;
    try {
      for (; i < other.size(); i++) {
        new(new_data + i) T(other[i]);
      }
    }
    catch (...) {
      while (i--) {
        new_data[i].~T();
      }
      operator delete(new_data);
      throw;
    }
    data_ = new_data;
    size_ = other.size();
    capacity_ = other.size();
  }

  vector &operator=(vector const &other) { // O(N) strong, swap-trick
    vector tmp(other);
    swap(tmp);
    return *this;
  }

  ~vector() noexcept { // O(N) nothrow
    clear();
    operator delete(data_);
  }

  T &operator[](size_t i) noexcept { // O(1) nothrow
    return data_[i];
  }

  T const &operator[](size_t i) const noexcept { // O(1) nothrow
    return data_[i];
  }

  T *data() noexcept { // O(1) nothrow
    return data_;
  }

  T const *data() const noexcept { // O(1) nothrow
    return data_;
  }

  size_t size() const noexcept { // O(1) nothrow
    return size_;
  }

  T &front() noexcept { // O(1) nothrow
    return data_[0];
  }

  T const &front() const noexcept { //O(1) nothrow
    return data_[0];
  }

  T &back() noexcept { // O(1) nothrow
    return data_[size_ - 1];
  }

  T const &back() const noexcept { // O(1) nothrow
    return data_[size_ - 1];
  }

  void push_back(T const &val) { // O(1)* strong
    if (size_ == capacity_) {
      push_back_realloc(val);
    } else {
      new(data_ + size_) T(val);
      size_++;
    }
  }

  void pop_back() noexcept { // O(1) nothrow
    data_[--size_].~T();
  }

  bool empty() const noexcept { // O(1) nothrow
    return size_ == 0;
  }

  size_t capacity() const noexcept { // O(1) nothrow
    return capacity_;
  }

  void reserve(size_t new_size) { // O(N) strong, swap-trick
    if (new_size <= capacity_) return;
    vector<T> tmp;
    tmp.data_ = static_cast<T *>(operator new(new_size * sizeof(T)));
    tmp.capacity_ = new_size;
    for (size_t i = 0; i < size_; i++) {
      tmp.push_back(data_[i]);
    }
    swap(tmp);
  }

  void shrink_to_fit() { // O(N) strong, swap-trick
    if (size_ == capacity_) return;
    vector<T> tmp(*this);
    swap(tmp);
  }

  void clear() noexcept { // O(N) nothrow
    while (size_) {
      pop_back();
    }
  }

  void swap(vector &other) noexcept { // O(1) nothrow
    using std::swap;
    swap(data_, other.data_);
    swap(size_, other.size_);
    swap(capacity_, other.capacity_);
  }

  iterator begin() noexcept { // O(1) nothrow
    return data_;
  }

  iterator end() noexcept { // O(1) nothrow
    return data_ + size_;
  }

  const_iterator begin() const noexcept { // O(1) nothrow
    return data_;
  }

  const_iterator end() const noexcept { // O(1) nothrow
    return data_ + size_;
  }

  iterator insert(const_iterator pos, T const &val) { // O(N) weak
    size_t posInd = pos - data_;
    push_back(val);
    for (size_t i = size_ - 1; i != posInd; i--) {
      std::swap(data_[i], data_[i - 1]);
    }
    return begin() + posInd;
  }

  iterator erase(const_iterator pos) { // O(N) weak
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) { // O(N) weak
    size_t delta = last - first;
    for (size_t i = first - data_; i < size_ - delta; i++) {
      data_[i] = data_[i + delta];
    }
    while (delta--) {
      pop_back();
    }
    return const_cast<iterator>(first);
  }

 private:
  void push_back_realloc(T const &val) { // O(1)* strong, swap-trick
    vector<T> tmp;
    tmp.reserve(calc_new_capacity());
    for (int i = 0; i < size_; i++) {
      tmp.push_back(data_[i]);
    }
    tmp.push_back(val);
    swap(tmp);
  }

  size_t calc_new_capacity() {
    return capacity_ == 0 ? 1 : capacity_ * 2;
  }

 private:
  T *data_;
  size_t size_;
  size_t capacity_;
};

#endif // VECTOR_H