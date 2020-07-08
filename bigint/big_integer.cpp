#include "big_integer.h"

#include <cstring>
#include <climits>
#include <functional>

const big_integer ZERO(0);

big_integer::big_integer() : sign_(false), data_({0}) {}

big_integer::big_integer(int a) : sign_(a < 0) {
  data_.push_back(a == INT_MIN ? static_cast<uint32_t>(INT_MAX) + 1 : static_cast<uint32_t>(std::abs(a)));
}

big_integer::big_integer(std::string const &str) {
  big_integer res;
  size_t i = 0;
  sign_ = false;
  if (str[0] == '+' || str[0] == '-') {
    sign_ = str[0] == '-';
    i++;
  }
  for (; i < str.size(); i++) {
    res = res * 10 + (str[i] - '0');
  }
  data_ = res.data_;
  shrink();
}

bool big_integer::overflow_add(uint32_t a, uint32_t b, bool carry) {
  return (a > MAX_VALUE - b) || (carry && (a + b == MAX_VALUE));
}

uint32_t big_integer::simple_overflow(uint64_t x) {
  return uint32_t(x >> BASE);
}

void big_integer::shrink() {
  while (data_.size() > 1 && data_.back() == 0) {
    data_.pop_back();
  }
  if (data_.size() == 1 && data_.back() == 0) {
    sign_ = false;
  }
}

int big_integer::compare_abs(big_integer const &a, big_integer const &b) {
  if (a.data_.size() != b.data_.size()) {
    return a.data_.size() < b.data_.size() ? -1 : 1;
  }
  for (size_t i = a.data_.size(); i > 0; i--) {
    if (a.data_[i - 1] != b.data_[i - 1]) {
      return a.data_[i - 1] < b.data_[i - 1] ? -1 : 1;
    }
  }
  return 0;
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
  if (sign_ != rhs.sign_) {
    return *this = (!sign_ ? (*this - -rhs) : (rhs - (-*this)));
  }
  size_t size_l = data_.size(), size_r = rhs.data_.size();
  size_t size_result = std::max(size_l, size_r);
  data_.resize(size_result);
  uint32_t carry = 0;
  for (size_t i = 0; i < size_result; i++) {
    uint32_t rhs_digit = i < size_r ? rhs.data_[i] : 0;
    uint64_t tmp = static_cast<uint64_t>(carry) + data_[i] + rhs_digit;
    if (overflow_add(data_[i], rhs_digit, carry)) {
      tmp &= MAX_VALUE;
      carry = 1;
    } else {
      carry = 0;
    }
    data_[i] = tmp;
  }
  data_.push_back(carry);
  shrink();
  return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
  if (sign_ != rhs.sign_) {
    return *this = (!sign_ ? *this + (-rhs) : -(-*this + rhs));
  } else if (sign_) {
    return *this = (-rhs - (-*this));
  }
  if (*this < rhs) {
    return *this = -(rhs - *this);
  }
  uint32_t carry = 0;
  for (size_t i = 0; i < rhs.data_.size() || carry; i++) {
    int64_t tmp;
    uint32_t rhs_digit = i < rhs.data_.size() ? rhs.data_[i] : 0;
    tmp = static_cast<int64_t>(data_[i]) - carry - rhs_digit;
    carry = 0;
    if (tmp < 0) {
      carry = 1;
      tmp += FULL_DIGIT;
    }
    data_[i] = tmp;
  }
  shrink();
  return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
  big_integer res;
  size_t size_1 = data_.size(),
      size_2 = rhs.data_.size(),
      size_result = size_1 + size_2 + 1;
  res.data_.resize(size_result, 0);
  for (size_t i = 0; i < size_1; i++) {
    uint64_t carry = 0;
    for (size_t j = 0; j < size_2 || carry; j++) {
      uint64_t tmp;
      uint32_t rhs_digit = j < rhs.data_.size() ? rhs.data_[j] : 0;
      uint32_t data_digit = i < data_.size() ? data_[i] : 0;
      tmp = res.data_[i + j] + static_cast<uint64_t>(data_digit) * rhs_digit + carry;
      res.data_[i + j] = tmp & MAX_VALUE;
      carry = simple_overflow(tmp);
    }
  }
  res.sign_ = (rhs.sign_ != sign_);
  res.shrink();
  return *this = res;
}

big_integer big_integer::product(big_integer y, uint32_t k) {
  uint32_t carry = 0;
  for (unsigned int &i : y.data_) {
    uint64_t tmp = static_cast<uint64_t>(i) * k + carry;
    i = tmp & MAX_VALUE;
    carry = simple_overflow(tmp);
  }
  y.data_.push_back(carry);
  y.shrink();
  return y;
}

/*
big_integer big_integer::product(big_integer y, uint32_t k) {
  uint64_t carry = 0;
  for (unsigned int &i : y.data_) {
    carry += static_cast<uint64_t>(i) * k;
    i = carry & MAX_VALUE;
    carry >>= BASE;
  }
  y.data_.push_back(static_cast<uint32_t>(carry));
  y.shrink();
  return y;
}
*/

big_integer big_integer::quotient(big_integer y, uint32_t k) {
  uint64_t carry = 0;
  for (size_t i = y.data_.size(); i > 0; i--) {
    uint64_t tmp = (carry << BASE) + y.data_[i - 1];
    y.data_[i - 1] = tmp / k;
    carry = tmp % k;
  }
  y.shrink();
  return y;
}

uint32_t big_integer::remainder(big_integer y, uint32_t k) {
  uint64_t carry = 0;
  for (size_t i = y.data_.size(); i > 0; i--) {
    carry = ((carry << BASE) + y.data_[i - 1]) % k;
  }
  return carry;
}

uint32_t big_integer::trial(uint64_t a, uint64_t b, uint64_t c) {
  return std::min(((a << BASE) + b) / c, static_cast<uint64_t>(MAX_VALUE));
}

bool big_integer::smaller(big_integer &r, big_integer &dq, uint32_t k, uint32_t m) {
  for (size_t i = m; i > 0; i--) {
    uint32_t dq_digit = i < dq.data_.size() ? dq.data_[i] : 0;
    if (r.data_[i - 1 + k] != dq_digit) {
      return r.data_[i - 1 + k] < dq_digit;
    }
  }
  return false;
}

void big_integer::difference(big_integer &r, big_integer &dq, uint32_t k, uint32_t m) {
  uint32_t borrow = 0;
  for (size_t i = 0; i < m; i++) {
    uint32_t dq_digit = i < dq.data_.size() ? dq.data_[i] : 0;
    int64_t diff = static_cast<int64_t>(r.data_[i + k - 1]) - dq_digit - borrow;
    borrow = 0;
    if (diff < 0) {
      borrow = 1;
      diff += FULL_DIGIT;
    }
    r.data_[i + k - 1] = diff;
  }
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
  int comparing = compare_abs(*this, rhs);
  if (comparing == -1) {
    return *this = 0;
  }
  sign_ = (rhs.sign_ != sign_);
  if (rhs.data_.size() == 1) {
    return *this = quotient(*this, rhs.data_[0]);
  }
  uint32_t f = FULL_DIGIT / (rhs.data_.back() + 1);
  big_integer r, d;
  r = product(*this, f);
  r.data_.push_back(0);
  d = product(rhs, f);
  size_t n = r.data_.size(), m = d.data_.size();
  data_.resize(n - m);
  big_integer dq;
  for (size_t k = n - m; k > 0; k--) {
    uint32_t qt = trial(r.data_[k + m - 1], r.data_[k + m - 2], d.data_[m - 1]);
    dq = product(d, qt);
    while (smaller(r, dq, k, m)) {
      qt--;
      dq -= d;
    }
    data_[k - 1] = qt;
    difference(r, dq, k, m);
  }
  shrink();
  return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
  return *this -= (*this / rhs) * rhs;
}

big_integer big_integer::to_complementary(big_integer const &a) {
  if (!a.sign_) {
    return a;
  }
  big_integer res(a);
  res.sign_ = false;
  for (unsigned int &i : res.data_) {
    i = ~i;
  }
  res += 1;
  return res;
}

big_integer &big_integer::bitwise(big_integer const &rhs,
                                  const std::function<uint32_t(uint32_t, uint32_t)> &f) {
  big_integer left = to_complementary(*this),
      right = to_complementary(rhs);
  size_t new_size = std::max(left.data_.size(), right.data_.size());
  big_integer res;
  res.data_.resize(new_size);

  uint32_t addition_1 = sign_ ? MAX_VALUE : 0,
            addition_2 = rhs.sign_ ? MAX_VALUE : 0;
  for (size_t i = 0; i < new_size; i++) {
    uint32_t left_digit = i < left.data_.size() ? left.data_[i] : addition_1;
    uint32_t right_digit = i < right.data_.size() ? right.data_[i] : addition_2;
    res.data_[i] = f(left_digit, right_digit);
  }
  res.sign_ = f(sign_, rhs.sign_);
  res.shrink();
  if (res.sign_) {
    res = to_complementary(res);
    res.sign_ = f(sign_, rhs.sign_);
  }
  res.shrink();
  return *this = res;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
  return bitwise(rhs, [](uint32_t a, uint32_t b) { return a & b; });
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
  return bitwise(rhs, [](uint32_t a, uint32_t b) { return a | b; });
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
  return bitwise(rhs, [](uint32_t a, uint32_t b) { return a ^ b; });
}

big_integer &big_integer::operator<<=(int rhs) {
  uint32_t limbs_cnt = rhs / BASE;
  uint32_t digit_cnt = rhs % BASE;
  uint32_t mod = 1u << digit_cnt;
  big_integer shifted = *this * mod;
  size_t new_size = shifted.data_.size() + limbs_cnt;

  data_.resize(new_size);
  for (size_t i = 0; i < limbs_cnt; ++i) {
    data_[i] = 0;
  }
  for (size_t i = 0; i < shifted.data_.size(); i++) {
    data_[i + limbs_cnt] = shifted.data_[i];
  }
  shrink();
  return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
  uint32_t limbs_cnt = rhs / BASE;
  uint32_t digit_cnt = rhs % BASE;
  uint32_t mod = 1u << digit_cnt;
  big_integer shifted = *this / mod;
  size_t new_size = shifted.data_.size() - limbs_cnt;

  if (new_size <= 0) {
    return *this = ZERO;
  }
  data_.resize(new_size);
  for (size_t i = 0; i < new_size; i++) {
    data_[i] = shifted.data_[i + limbs_cnt];
  }
  if (sign_) {
    *this -= 1;
  }
  shrink();
  return *this;
}

big_integer big_integer::operator+() const {
  return *this;
}

big_integer big_integer::operator-() const {
  big_integer tmp(*this);
  if (tmp != ZERO) {
    tmp.sign_ = !sign_;
  }
  return tmp;
}

big_integer big_integer::operator~() const {
  return -*this - 1;
}

big_integer &big_integer::operator++() {
  return *this += 1;
}

big_integer big_integer::operator++(int) {
  big_integer r = *this;
  ++*this;
  return r;
}

big_integer &big_integer::operator--() {
  return *this -= 1;
}

big_integer big_integer::operator--(int) {
  big_integer r = *this;
  --*this;
  return r;
}

big_integer operator+(big_integer a, big_integer const &b) {
  return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
  return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
  return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
  return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
  return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
  return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
  return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
  return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
  return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
  return a >>= b;
}

bool operator==(big_integer const &a, big_integer const &b) {
  return a.sign_ == b.sign_ && a.data_ == b.data_;
}

bool operator!=(big_integer const &a, big_integer const &b) {
  return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
  if (a.sign_ != b.sign_) {
    return a.sign_ && !b.sign_;
  }
  big_integer first = !a.sign_ ? a : b;
  big_integer second = !a.sign_ ? b : a;
  return (big_integer::compare_abs(first, second)) == -1;
}

bool operator>(big_integer const &a, big_integer const &b) {
  return !(a < b) && (a != b);
}

bool operator<=(big_integer const &a, big_integer const &b) {
  return (a < b) || (a == b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
  return (a > b) || (a == b);
}

std::string to_string(big_integer const &a) {
  if (a == ZERO) {
    return "0";
  }
  big_integer cur(a);
  std::string ans;
  while (cur != 0) {
    ans += static_cast<char>(big_integer::remainder(cur, 10)) + '0';
    cur = big_integer::quotient(cur, 10);
  }
  if (a.sign_) {
    ans += '-';
  }
  reverse(ans.begin(), ans.end());
  return ans;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
  return s << to_string(a);
}
