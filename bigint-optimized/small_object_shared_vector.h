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

  void swap_data(small_object_shared_vector<T> &a, small_object_shared_vector<T> &b) {
    shared_vector<T> tmp(b.dynamic_data_);
    b.dynamic_data_.~shared_vector();
    safe_copy_static(a.static_data_, b.static_data_, a.size_);
    new(&a.dynamic_data_) shared_vector<T>(tmp);
  }

  void safe_copy_static(T const *from, T *to, size_t size) {
    size_t i = 0;
    try {
      for (; i < size; i++) {
        new(to + i) T(from[i]);
      }
    } catch (...) {
      clear_static(to, i, 0);
      throw;
    }
  }

  void safe_initialize_with_static(T *to, size_t first, size_t last, T const &val) {
    size_t i = first;
    try {
      for (; i < last; i++) {
        new(to + i) T(val);
      }
    } catch (...) {
      clear_static(to, i, first);
      throw;
    }
  }

  void clear_static(T *to, size_t first, size_t last) {
    for (size_t i = first; i > last + 1; i--) {
      to[i - 1].~T();
    }
  }

 public:
  small_object_shared_vector() : size_(0), is_small(true) {}

  small_object_shared_vector(small_object_shared_vector<T> const &other)
      : size_(other.size_), is_small(other.is_small) {
    if (is_small) {
      safe_copy_static(other.static_data_, static_data_, size_);
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

  T &operator[](size_t i) {
    if (is_small) {
      return static_data_[i];
    } else {
      return dynamic_data_[i];
    }
  }

  const T &operator[](size_t i) const {
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
        if (is_small) {
          swap(static_data_, tmp.static_data_);
        } else {
          swap(dynamic_data_, tmp.dynamic_data_);
        }
      } else {
        if (is_small) {
          swap_data(*this, tmp);
        } else {
          swap_data(tmp, *this);
        }
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
        new(static_data_ + size_) T(val);
        size_++;
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

  void resize(size_t n, T const &val = T()) {
    if (size_ == n) {
      return;
    }
    if (n > size_) {
      if (is_small && n > MAX_SMALL_SIZE) {
        from_small_to_big();
      }
      if (is_small) {
        safe_initialize_with_static(static_data_, size_, size_ + n, val);
      } else {
        dynamic_data_.resize(n, val);
      }
    } else {
      if (is_small) {
        for (size_t i = size_; i > n; i--) {
          static_data_[i - 1].~T();
        }
      } else {
        dynamic_data_.resize(n, val);
      }
    }
    size_ = n;
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
};

#ifndef EXAM__SMALL_OBJECT_SHARED_VECTOR_H_
#define EXAM__SMALL_OBJECT_SHARED_VECTOR_H_

#endif //EXAM__SMALL_OBJECT_SHARED_VECTOR_H_
