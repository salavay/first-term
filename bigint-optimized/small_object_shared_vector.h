#include "shared.h"

template<typename T>
class small_object_shared_vector {
 private:
  constexpr static size_t MAX_SMALL_SIZE = sizeof(shared_vector<T>) / sizeof(T);
  size_t size_;
  bool is_small;

  union {
    T static_data_[MAX_SMALL_SIZE];
    shared_vector<T> dynamic_data_;
  };

  void from_small_to_big() {
    shared_vector<T> tmp(static_data_, static_data_ + size_);
    destroy_small();
    new(&dynamic_data_) shared_vector<T>(tmp);
    is_small = false;
  }

 public:
  small_object_shared_vector() : size_(0), is_small(true) {}

  small_object_shared_vector(small_object_shared_vector<T> const &other)
      : size_(other.size_), is_small(other.is_small) {
    if (is_small) {
      for (size_t i = 0; i < size_; i++) {
        new(static_data_ + i) T(other.static_data_[i]);
      }
    } else {
      new(&dynamic_data_) shared_vector<T>(other.dynamic_data_);
    }
  }

  ~small_object_shared_vector() {
    if (is_small) {
      destroy_small();
    } else {
      dynamic_data_.~shared_vector();
    }
  }

  void destroy_small() {
    for (size_t i = size_; i > 0; i--) {
      static_data_[i - 1].~T();
    }
  }

  T &operator[](size_t const &i) {
    if (is_small) {
      return static_data_[i];
    } else {
      return dynamic_data_[i];
    }
  }

  const T &operator[](size_t const &i) const {
    if (is_small) {
      return static_data_[i];
    } else {
      return dynamic_data_[i];
    }
  }

  small_object_shared_vector &operator=(small_object_shared_vector<T> const &other) {
    if (this != &other) {
      using std::swap;
      small_object_shared_vector<T> tmp(other);
      if (is_small == tmp.is_small) {
        is_small ? swap(static_data_, tmp.static_data_) :
        swap(dynamic_data_, tmp.dynamic_data_);
      } else {
        is_small ? swap_data(*this, tmp) :
        swap_data(tmp, *this);
      }
      swap(is_small, tmp.is_small);
      swap(size_, tmp.size_);
    }
    return *this;
  }

  void push_back(T const &val) {
    if (is_small) {
      if (size_ == MAX_SMALL_SIZE) {
        from_small_to_big();
      } else {
        size_++;
        new(static_data_ + size_ - 1) T(val);
        return;
      }
    }
    dynamic_data_.push_back(val);
    size_++;
  }

  void pop_back() {
    if (is_small) {
      static_data_[size_ - 1].~T();
    } else {
      dynamic_data_.pop_back();
    }
    size_--;
  }

  void resize(size_t const &n) {
    if (size_ == n) {
      return;
    }
    if (n > size_) {
      if (is_small && n > MAX_SMALL_SIZE) {
        from_small_to_big();
      }
      if (is_small) {
        for (size_t i = size_; i < size_ + n; i++) {
          new(static_data_ + i) T();
        }
      } else {
        dynamic_data_.resize(n);
      }
    } else {
      if (is_small) {
        for (size_t i = size_; i > n; i--) {
          static_data_[i - 1].~T();
        }
      } else {
        dynamic_data_.resize(n);
      }
    }
    size_ = n;
  }

  void resize(size_t const &n, T const &val) {
    size_t saved_size = size_;
    resize(n);
    if (n <= saved_size) {
      return;
    }
    if (is_small) {
      for (size_t i = saved_size; i < size_; i++) {
        new(static_data_ + i) T(val);
      }
    } else {
      for (size_t i = saved_size; i < size_; i++) {
        dynamic_data_[i] = val;
      }
    }

  }

  T const &back() const {
    if (is_small) {
      return static_data_[size_ - 1];
    } else {
      return dynamic_data_.back();
    }
  }

  T *begin() {
    if (is_small) {
      return static_data_;
    } else {
      dynamic_data_.begin();
    }
  }

  T *end() {
    return begin() + size_;
  }

  size_t size() const {
    return size_;
  }

  void swap_data(small_object_shared_vector<T> &a, small_object_shared_vector<T> &b) {
    shared_vector<T> tmp(b.dynamic_data_);
    b.dynamic_data_.~shared_vector();
    for (size_t i = 0; i < a.size_; i++) {
      new(b.static_data_ + i) T(a.static_data_[i]);
    }
    new(&a.dynamic_data_) shared_vector<T>(tmp);
  }
};

#ifndef EXAM__SMALL_OBJECT_SHARED_VECTOR_H_
#define EXAM__SMALL_OBJECT_SHARED_VECTOR_H_

#endif //EXAM__SMALL_OBJECT_SHARED_VECTOR_H_
