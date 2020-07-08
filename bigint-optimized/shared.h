#include <vector>

template<typename T>
class shared_vector {
 private:
  struct data_type {
    std::vector<T> data_impl;
    size_t ref_counter;
    explicit data_type(std::vector<T> const &data_impl) : data_impl(data_impl), ref_counter(1) {};
    data_type(T *first, T *last) : data_impl(first, last), ref_counter(1) {}
    ~data_type() = default;
  };

  data_type *data_;

  void own() {
    if (data_->ref_counter > 1) {
      unshare();
      data_ = new data_type(data_->data_impl);
    }
  }

  void unshare() {
    data_->ref_counter--;
    if (data_->ref_counter == 0) {
      delete data_;
    }
  }

 public:
  shared_vector() {
    data_ = new data_type(std::vector<T>());
  }

  shared_vector(shared_vector<T> const &other) {
    data_ = other.data_;
    data_->ref_counter++;
  }

  shared_vector(T *first, T *last) {
    data_ = new data_type(first, last);
  }

  ~shared_vector() {
    unshare();
  }

  T &operator[](size_t i) {
    own();
    return data_->data_impl[i];
  }

  T const &operator[](size_t i) const {
    return data_->data_impl[i];
  }

  shared_vector &operator=(shared_vector<T> const &other) {
    if (data_ != other.data_) {
      unshare();
      data_ = other.data_;
      data_->ref_counter++;
    }
    return *this;
  }

  void push_back(T const &val) {
    own();
    data_->data_impl.push_back(val);
  }

  void pop_back() {
    own();
    data_->data_impl.pop_back();
  }

  void resize(size_t n) {
    own();
    return data_->data_impl.resize(n);
  }

  T const &back() const {
    return data_->data_impl.back();
  }

  T *begin() {
    own();
    return data_->data_impl.begin();
  }

  T *end() {
    own();
    return data_->data_impl.end();
  }

  size_t size_() const {
    return data_->data_impl.size();
  }
};

#ifndef EXAM__SHARED_H_
#define EXAM__SHARED_H_

#endif //EXAM__SHARED_H_
