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

  vector(vector const &other) {
    T *newData = nullptr;
    if (other.data_ && other.size() != 0) {
      newData = static_cast<T *>(operator new(other.size() * sizeof(T)));
    }
    size_t i = 0;
    try {
      for (; i < other.size(); i++) {
        new(newData + i) T();
        newData[i] = other[i];
      }
    }
    catch (...) {
      while (i--) {
        newData[i].~T();
      }
      operator delete(newData);
      throw;
    }
    data_ = newData;
    size_ = other.size();
    capacity_ = other.size();
  }// O(N) strong

  vector &operator=(vector const &other) {
    vector tmp(other);
    swap(tmp);
    return *this;
  } // O(N) strong, swap-trick

  ~vector() noexcept {
    if (data_) {
      for (size_t i = size_; i--;) {
        data_[i].~T();
      }
      operator delete(data_);
    }
  } // O(N) nothrow

  T &operator[](size_t i) noexcept {
    return data_[i];
  } // O(1) nothrow

  T const &operator[](size_t i) const noexcept {
    return data_[i];
  } // O(1) nothrow

  T *data() noexcept {
    return data_;
  }// O(1) nothrow

  T const *data() const noexcept {
    return data_;
  }// O(1) nothrow

  size_t size() const noexcept {
    return size_;
  }// O(1) nothrow

  T &front() noexcept {
    return data_[0];
  }// O(1) nothrow

  T const &front() const noexcept {
    return data_[0];
  }//O(1) nothrow

  T &back() noexcept {
    return data_[size_ - 1];
  }// O(1) nothrow

  T const &back() const noexcept {
    return data_[size_ - 1];
  }// O(1) nothrow

  void push_back(T const &val) {
    T valSaved = val;
    if (size_ == capacity_) {
      push_back_realloc(valSaved);
    } else {
      new(data_ + size_) T(valSaved);
      size_++;
    }
  }// O(1)* strong

  void pop_back() noexcept {
    data_[--size_].~T();
  }// O(1) nothrow

  bool empty() const noexcept {
    return size_ == 0;
  }// O(1) nothrow

  size_t capacity() const noexcept {
    return capacity_;
  }// O(1) nothrow

  void reserve(size_t new_size) {
    if (new_size <= capacity_) return;
    vector<T> tmp;
    tmp.data_ = static_cast<T *>(operator new(new_size * sizeof(T)));
    tmp.capacity_ = new_size;
    for (size_t i = 0; i < size_; i++) {
      tmp.push_back(data_[i]);
    }
    swap(tmp);
  }// O(N) strong, swap-trick
  void shrink_to_fit() {
    if (size_ == capacity_) return;
    vector<T> tmp(*this);
    swap(tmp);
  }// O(N) strong, swap-trick

  void clear() noexcept {
    for (size_t i = 0; i < size_; i++) {
      data_[i].~T();
    }
    size_ = 0;
  }   // O(N) nothrow

  void swap(vector &other) noexcept {
    using std::swap;
    swap(data_, other.data_);
    swap(size_, other.size_);
    swap(capacity_, other.capacity_);
  }// O(1) nothrow

  iterator begin() noexcept {
    return data_;
  }// O(1) nothrow

  iterator end() noexcept {
    return data_ + size_;
  }// O(1) nothrow

  const_iterator begin() const noexcept {
    return data_;
  }// O(1) nothrow

  const_iterator end() const noexcept {
    return data_ + size_;
  }// O(1) nothrow

  iterator insert(const_iterator pos, T const &val) {
    size_t posInd = pos - data_;
    if (posInd == size_) {
      push_back(val);
      return data_ + size_;
    }
    increase_capacity();
    new(data_ + size_)  T(data_[size_ - 1]);
    for (size_t i = size_ - 1; i != posInd; i--) {
      data_[i] = data_[i - 1];
    }
    size_++;
    data_[posInd] = val;
    return begin() + posInd;
  }// O(N) weak

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }// O(N) weak

  iterator erase(const_iterator first, const_iterator last) {
    size_t delta = last - first;
    for (size_t i = first - data_; i < size_ - delta; i++) {
      data_[i] = data_[i + delta];
    }
    for (size_t i = size_; i-- > size_ - delta;) {
      data_[i].~T();
    }
    size_ -= delta;
    return const_cast<iterator>(first);
  }// O(N) weak

 private:
  void increase_capacity() {
    if (size_ >= capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }
  }// O(n) strong

  void push_back_realloc(T const &val) {
    size_t new_capacity = calc_new_capacity();
    T *tmp = static_cast<T *>(operator new(new_capacity * sizeof(T)));
    size_t new_size = size_ + 1;
    size_t i = size_;
    try {
      new(tmp + i) T(val);
      while (i--) {
        new(tmp + i) T(data_[i]);
      }
    }
    catch (...) {
      for (i++; i < new_size; i++) {
        tmp[i].~T();
      }
      operator delete(tmp);
      throw;
    }
    clear();
    operator delete(data_);
    data_ = tmp;
    size_ = new_size;
    capacity_ = new_capacity;
  }// O(1)* strong

  size_t calc_new_capacity() {
    return capacity_ == 0 ? 1 : capacity_ * 2;
  }

 private:
  T *data_;
  size_t size_;
  size_t capacity_;
};

#endif // VECTOR_H