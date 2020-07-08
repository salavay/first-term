#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <iosfwd>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <functional>
#include "small_object_shared_vector.h"

struct big_integer
{
  big_integer();
  big_integer(big_integer const& other) = default;
  big_integer(int a);
  explicit big_integer(std::string const& str);
  ~big_integer() = default;

  big_integer& operator=(big_integer const& other) = default;

  big_integer& operator+=(big_integer const& rhs);
  big_integer& operator-=(big_integer const& rhs);
  big_integer& operator*=(big_integer const& rhs);
  big_integer& operator/=(big_integer const& rhs);
  big_integer& operator%=(big_integer const& rhs);

  big_integer& operator&=(big_integer const& rhs);
  big_integer& operator|=(big_integer const& rhs);
  big_integer& operator^=(big_integer const& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(big_integer const& a, big_integer const& b);
  friend bool operator!=(big_integer const& a, big_integer const& b);
  friend bool operator<(big_integer const& a, big_integer const& b);
  friend bool operator>(big_integer const& a, big_integer const& b);
  friend bool operator<=(big_integer const& a, big_integer const& b);
  friend bool operator>=(big_integer const& a, big_integer const& b);

  friend std::string to_string(big_integer const& a);

 private:
  using double_data_type = uint64_t;
  using storage = small_object_shared_vector<uint32_t>;
  bool sign_;
  storage data_;
  constexpr static uint32_t MAX_VALUE = UINT32_MAX;
  constexpr static uint32_t BASE = 32;
  constexpr static uint64_t FULL_DIGIT = (1ULL << 32u);
  static bool overflow_add(uint32_t a, uint32_t b, bool carry);
  void shrink();
  static int compare_abs(big_integer const &a, big_integer const &b);
  static uint32_t simple_overflow(uint64_t);
  static big_integer to_complementary(big_integer const &a);
  static big_integer product(big_integer y, uint32_t k);
  static big_integer quotient(big_integer y, uint32_t k);
  static uint32_t remainder(big_integer y, uint32_t k);
  static uint32_t trial(uint64_t a, uint64_t b, uint64_t c);
  static bool smaller(big_integer &r, big_integer &dq, uint32_t k, uint32_t m);
  static void difference(big_integer &r, big_integer &dq, uint32_t k, uint32_t m);

  big_integer& bitwise(big_integer const& rhs,
                       const std::function<uint32_t(uint32_t, uint32_t)>& f);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif // BIG_INTEGER_H