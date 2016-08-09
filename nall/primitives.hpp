#pragma once

#include <nall/serializer.hpp>
#include <nall/traits.hpp>

namespace nall {

struct Boolean {
  inline Boolean() : data(false) {}
  template<typename T> inline Boolean(const T& value) : data(value) {}

  inline operator bool() const { return data; }
  template<typename T> inline auto& operator=(const T& value) { data = value; return *this; }

  inline auto flip() { return data ^= 1; }
  inline auto raise() { return data == 0 ? data = 1, true : false; }
  inline auto lower() { return data == 1 ? data = 0, true : false; }

  inline auto serialize(serializer& s) { s(data); }

private:
  bool data;
};

template<uint Bits> struct Natural {
  using type =
    type_if<expression<Bits <=  8>,  uint8_t,
    type_if<expression<Bits <= 16>, uint16_t,
    type_if<expression<Bits <= 32>, uint32_t,
    type_if<expression<Bits <= 64>, uint64_t,
    void>>>>;

  enum : type { Mask = ~0ull >> (64 - Bits) };

  inline Natural() : data(0) {}
  template<typename T> inline Natural(const T& value) { set(value); }

  inline operator type() const { return data; }
  template<typename T> inline auto& operator=(const T& value) { set(value); return *this; }

  inline auto operator++(int) { type value = data; set(data + 1); return value; }
  inline auto operator--(int) { type value = data; set(data - 1); return value; }

  inline auto& operator++() { set(data + 1); return *this; }
  inline auto& operator--() { set(data - 1); return *this; }

  inline auto& operator &=(const type value) { set(data  & value); return *this; }
  inline auto& operator |=(const type value) { set(data  | value); return *this; }
  inline auto& operator ^=(const type value) { set(data  ^ value); return *this; }
  inline auto& operator<<=(const type value) { set(data << value); return *this; }
  inline auto& operator>>=(const type value) { set(data >> value); return *this; }
  inline auto& operator +=(const type value) { set(data  + value); return *this; }
  inline auto& operator -=(const type value) { set(data  - value); return *this; }
  inline auto& operator *=(const type value) { set(data  * value); return *this; }
  inline auto& operator /=(const type value) { set(data  / value); return *this; }
  inline auto& operator %=(const type value) { set(data  % value); return *this; }

  inline auto serialize(serializer& s) { s(data); }

  struct Reference {
    inline Reference(Natural& source, uint lo, uint hi) : source(source), Lo(lo), Hi(hi) {}

    inline auto get() const -> type {
      const type RangeBits = Hi - Lo + 1;
      const type RangeMask = (((1ull << RangeBits) - 1) << Lo) & Mask;
      return (source & RangeMask) >> Lo;
    }

    inline auto& set(const type value) {
      const type RangeBits = Hi - Lo + 1;
      const type RangeMask = (((1ull << RangeBits) - 1) << Lo) & Mask;
      source = (source & ~RangeMask) | ((value << Lo) & RangeMask);
      return *this;
    }

    inline operator type() const { return get(); }
    inline auto& operator  =(const type value) { return set(         value); }
    inline auto& operator &=(const type value) { return set(get()  & value); }
    inline auto& operator |=(const type value) { return set(get()  | value); }
    inline auto& operator ^=(const type value) { return set(get()  ^ value); }
    inline auto& operator<<=(const type value) { return set(get() << value); }
    inline auto& operator>>=(const type value) { return set(get() >> value); }
    inline auto& operator +=(const type value) { return set(get()  + value); }
    inline auto& operator -=(const type value) { return set(get()  - value); }
    inline auto& operator *=(const type value) { return set(get()  * value); }
    inline auto& operator /=(const type value) { return set(get()  / value); }
    inline auto& operator %=(const type value) { return set(get()  % value); }
    inline auto& operator++(int) { auto value = get(); set(value + 1); return value; }
    inline auto& operator--(int) { auto value = get(); set(value - 1); return value; }
    inline auto& operator++() { return set(get() + 1); }
    inline auto& operator--() { return set(get() - 1); }

  private:
    Natural& source;
    const type Lo;
    const type Hi;
  };

  inline auto bits(uint lo, uint hi) -> Reference { return {*this, lo < hi ? lo : hi, hi > lo ? hi : lo}; }
  inline auto bit(uint index) -> Reference { return {*this, index, index}; }
  inline auto byte(uint index) -> Reference { return {*this, index * 8 + 0, index * 8 + 7}; }

  inline auto bits(uint lo, uint hi) const -> const Reference { return {(Natural&)*this, lo < hi ? lo : hi, hi > lo ? hi : lo}; }
  inline auto bit(uint index) const -> const Reference { return {(Natural&)*this, index, index}; }
  inline auto byte(uint index) const -> const Reference { return {(Natural&)*this, index * 8 + 0, index * 8 + 7}; }

  inline auto clamp(uint bits) -> uintmax {
    const uintmax b = 1ull << (bits - 1);
    const uintmax m = b * 2 - 1;
    return data < m ? data : m;
  }

  inline auto clip(uint bits) -> uintmax {
    const uintmax b = 1ull << (bits - 1);
    const uintmax m = b * 2 - 1;
    return data & m;
  }

private:
  auto set(type value) -> void {
    data = value & Mask;
  }

  type data;
};

template<uint Bits> struct Integer {
  using type =
    type_if<expression<Bits <=  8>,  int8_t,
    type_if<expression<Bits <= 16>, int16_t,
    type_if<expression<Bits <= 32>, int32_t,
    type_if<expression<Bits <= 64>, int64_t,
    void>>>>;
  using utype = typename Natural<Bits>::type;

  enum : utype { Mask = ~0ull >> (64 - Bits), Sign = 1ull << (Bits - 1) };

  inline Integer() : data(0) {}
  template<typename T> inline Integer(const T& value) { set(value); }

  inline operator type() const { return data; }
  template<typename T> inline auto& operator=(const T& value) { set(value); return *this; }

  inline auto operator++(int) { type value = data; set(data + 1); return value; }
  inline auto operator--(int) { type value = data; set(data - 1); return value; }

  inline auto& operator++() { set(data + 1); return *this; }
  inline auto& operator--() { set(data - 1); return *this; }

  inline auto& operator &=(const type value) { set(data  & value); return *this; }
  inline auto& operator |=(const type value) { set(data  | value); return *this; }
  inline auto& operator ^=(const type value) { set(data  ^ value); return *this; }
  inline auto& operator<<=(const type value) { set(data << value); return *this; }
  inline auto& operator>>=(const type value) { set(data >> value); return *this; }
  inline auto& operator +=(const type value) { set(data  + value); return *this; }
  inline auto& operator -=(const type value) { set(data  - value); return *this; }
  inline auto& operator *=(const type value) { set(data  * value); return *this; }
  inline auto& operator /=(const type value) { set(data  / value); return *this; }
  inline auto& operator %=(const type value) { set(data  % value); return *this; }

  inline auto serialize(serializer& s) { s(data); }

  struct Reference {
    inline Reference(Integer& source, uint lo, uint hi) : source(source), Lo(lo), Hi(hi) {}

    inline auto get() const -> utype {
      const type RangeBits = Hi - Lo + 1;
      const type RangeMask = (((1ull << RangeBits) - 1) << Lo) & Mask;
      return ((utype)source & RangeMask) >> Lo;
    }

    inline auto& set(const utype value) {
      const type RangeBits = Hi - Lo + 1;
      const type RangeMask = (((1ull << RangeBits) - 1) << Lo) & Mask;
      source = ((utype)source & ~RangeMask) | ((value << Lo) & RangeMask);
      return *this;
    }

    inline operator utype() const { return get(); }
    inline auto& operator  =(const utype value) { return set(         value); }
    inline auto& operator &=(const utype value) { return set(get()  & value); }
    inline auto& operator |=(const utype value) { return set(get()  | value); }
    inline auto& operator ^=(const utype value) { return set(get()  ^ value); }
    inline auto& operator<<=(const utype value) { return set(get() << value); }
    inline auto& operator>>=(const utype value) { return set(get() >> value); }
    inline auto& operator +=(const utype value) { return set(get()  + value); }
    inline auto& operator -=(const utype value) { return set(get()  - value); }
    inline auto& operator *=(const utype value) { return set(get()  * value); }
    inline auto& operator /=(const utype value) { return set(get()  / value); }
    inline auto& operator %=(const utype value) { return set(get()  % value); }
    inline auto& operator++(int) { auto value = get(); set(value + 1); return value; }
    inline auto& operator--(int) { auto value = get(); set(value - 1); return value; }
    inline auto& operator++() { return set(get() + 1); }
    inline auto& operator--() { return set(get() - 1); }

  private:
    Integer& source;
    const uint Lo;
    const uint Hi;
  };

  inline auto bits(uint lo, uint hi) -> Reference { return {*this, lo < hi ? lo : hi, hi > lo ? hi : lo}; }
  inline auto bit(uint index) -> Reference { return {*this, index, index}; }
  inline auto byte(uint index) -> Reference { return {*this, index * 8 + 0, index * 8 + 7}; }

  inline auto bits(uint lo, uint hi) const -> const Reference { return {(Integer&)*this, lo < hi ? lo : hi, hi > lo ? hi : lo}; }
  inline auto bit(uint index) const -> const Reference { return {(Integer&)*this, index, index}; }
  inline auto byte(uint index) const -> const Reference { return {(Integer&)*this, index * 8 + 0, index * 8 + 7}; }

  inline auto clamp(uint bits) -> intmax {
    const intmax b = 1ull << (bits - 1);
    const intmax m = b - 1;
    return data > m ? m : data < -b ? -b : data;
  }

  inline auto clip(uint bits) -> intmax {
    const uintmax b = 1ull << (bits - 1);
    const uintmax m = b * 2 - 1;
    return ((data & m) ^ b) - b;
  }

private:
  auto set(type value) -> void {
    data = ((value & Mask) ^ Sign) - Sign;
  }

  type data;
};

template<uint Bits> struct Real {
  using type =
    type_if<expression<Bits == 32>, float32_t,
    type_if<expression<Bits == 64>, float64_t,
  //type_if<expression<Bits == 80>, float80_t,
    void>>;

  inline Real() : data(0.0) {}
  template<typename T> inline Real(const T& value) : data((type)value) {}

  inline operator type() const { return data; }
  template<typename T> inline auto& operator=(const T& value) { data = (type)value; return *this; }

  inline auto operator++(int) { type value = data; ++data; return value; }
  inline auto operator--(int) { type value = data; --data; return value; }

  inline auto& operator++() { data++; return *this; }
  inline auto& operator--() { data--; return *this; }

  inline auto& operator+=(const type value) { data = data + value; return *this; }
  inline auto& operator-=(const type value) { data = data - value; return *this; }
  inline auto& operator*=(const type value) { data = data * value; return *this; }
  inline auto& operator/=(const type value) { data = data / value; return *this; }
  inline auto& operator%=(const type value) { data = data % value; return *this; }

  inline auto serialize(serializer& s) { s(data); }

private:
  type data;
};

using boolean = nall::Boolean;
using natural = nall::Natural<sizeof(uint) * 8>;
using integer = nall::Integer<sizeof( int) * 8>;
using real    = nall::Real<sizeof(double) * 8>;

using  natural1 = nall::Natural< 1>;
using  natural2 = nall::Natural< 2>;
using  natural3 = nall::Natural< 3>;
using  natural4 = nall::Natural< 4>;
using  natural5 = nall::Natural< 5>;
using  natural6 = nall::Natural< 6>;
using  natural7 = nall::Natural< 7>;
using  natural8 = nall::Natural< 8>;
using  natural9 = nall::Natural< 9>;
using natural10 = nall::Natural<10>;
using natural11 = nall::Natural<11>;
using natural12 = nall::Natural<12>;
using natural13 = nall::Natural<13>;
using natural14 = nall::Natural<14>;
using natural15 = nall::Natural<15>;
using natural16 = nall::Natural<16>;
using natural17 = nall::Natural<17>;
using natural18 = nall::Natural<18>;
using natural19 = nall::Natural<19>;
using natural20 = nall::Natural<20>;
using natural21 = nall::Natural<21>;
using natural22 = nall::Natural<22>;
using natural23 = nall::Natural<23>;
using natural24 = nall::Natural<24>;
using natural25 = nall::Natural<25>;
using natural26 = nall::Natural<26>;
using natural27 = nall::Natural<27>;
using natural28 = nall::Natural<28>;
using natural29 = nall::Natural<29>;
using natural30 = nall::Natural<30>;
using natural31 = nall::Natural<31>;
using natural32 = nall::Natural<32>;
using natural33 = nall::Natural<33>;
using natural34 = nall::Natural<34>;
using natural35 = nall::Natural<35>;
using natural36 = nall::Natural<36>;
using natural37 = nall::Natural<37>;
using natural38 = nall::Natural<38>;
using natural39 = nall::Natural<39>;
using natural40 = nall::Natural<40>;
using natural41 = nall::Natural<41>;
using natural42 = nall::Natural<42>;
using natural43 = nall::Natural<43>;
using natural44 = nall::Natural<44>;
using natural45 = nall::Natural<45>;
using natural46 = nall::Natural<46>;
using natural47 = nall::Natural<47>;
using natural48 = nall::Natural<48>;
using natural49 = nall::Natural<49>;
using natural50 = nall::Natural<50>;
using natural51 = nall::Natural<51>;
using natural52 = nall::Natural<52>;
using natural53 = nall::Natural<53>;
using natural54 = nall::Natural<54>;
using natural55 = nall::Natural<55>;
using natural56 = nall::Natural<56>;
using natural57 = nall::Natural<57>;
using natural58 = nall::Natural<58>;
using natural59 = nall::Natural<59>;
using natural60 = nall::Natural<60>;
using natural61 = nall::Natural<61>;
using natural62 = nall::Natural<62>;
using natural63 = nall::Natural<63>;
using natural64 = nall::Natural<64>;

using  integer1 = nall::Integer< 1>;
using  integer2 = nall::Integer< 2>;
using  integer3 = nall::Integer< 3>;
using  integer4 = nall::Integer< 4>;
using  integer5 = nall::Integer< 5>;
using  integer6 = nall::Integer< 6>;
using  integer7 = nall::Integer< 7>;
using  integer8 = nall::Integer< 8>;
using  integer9 = nall::Integer< 9>;
using integer10 = nall::Integer<10>;
using integer11 = nall::Integer<11>;
using integer12 = nall::Integer<12>;
using integer13 = nall::Integer<13>;
using integer14 = nall::Integer<14>;
using integer15 = nall::Integer<15>;
using integer16 = nall::Integer<16>;
using integer17 = nall::Integer<17>;
using integer18 = nall::Integer<18>;
using integer19 = nall::Integer<19>;
using integer20 = nall::Integer<20>;
using integer21 = nall::Integer<21>;
using integer22 = nall::Integer<22>;
using integer23 = nall::Integer<23>;
using integer24 = nall::Integer<24>;
using integer25 = nall::Integer<25>;
using integer26 = nall::Integer<26>;
using integer27 = nall::Integer<27>;
using integer28 = nall::Integer<28>;
using integer29 = nall::Integer<29>;
using integer30 = nall::Integer<30>;
using integer31 = nall::Integer<31>;
using integer32 = nall::Integer<32>;
using integer33 = nall::Integer<33>;
using integer34 = nall::Integer<34>;
using integer35 = nall::Integer<35>;
using integer36 = nall::Integer<36>;
using integer37 = nall::Integer<37>;
using integer38 = nall::Integer<38>;
using integer39 = nall::Integer<39>;
using integer40 = nall::Integer<40>;
using integer41 = nall::Integer<41>;
using integer42 = nall::Integer<42>;
using integer43 = nall::Integer<43>;
using integer44 = nall::Integer<44>;
using integer45 = nall::Integer<45>;
using integer46 = nall::Integer<46>;
using integer47 = nall::Integer<47>;
using integer48 = nall::Integer<48>;
using integer49 = nall::Integer<49>;
using integer50 = nall::Integer<50>;
using integer51 = nall::Integer<51>;
using integer52 = nall::Integer<52>;
using integer53 = nall::Integer<53>;
using integer54 = nall::Integer<54>;
using integer55 = nall::Integer<55>;
using integer56 = nall::Integer<56>;
using integer57 = nall::Integer<57>;
using integer58 = nall::Integer<58>;
using integer59 = nall::Integer<59>;
using integer60 = nall::Integer<60>;
using integer61 = nall::Integer<61>;
using integer62 = nall::Integer<62>;
using integer63 = nall::Integer<63>;
using integer64 = nall::Integer<64>;

using real32 = nall::Real<32>;
using real64 = nall::Real<64>;
//using real80 = nall::Real<80>;

}

using  int1 = nall::Integer< 1>;
using  int2 = nall::Integer< 2>;
using  int3 = nall::Integer< 3>;
using  int4 = nall::Integer< 4>;
using  int5 = nall::Integer< 5>;
using  int6 = nall::Integer< 6>;
using  int7 = nall::Integer< 7>;
using  int8 = nall::Integer< 8>;
using  int9 = nall::Integer< 9>;
using int10 = nall::Integer<10>;
using int11 = nall::Integer<11>;
using int12 = nall::Integer<12>;
using int13 = nall::Integer<13>;
using int14 = nall::Integer<14>;
using int15 = nall::Integer<15>;
using int16 = nall::Integer<16>;
using int17 = nall::Integer<17>;
using int18 = nall::Integer<18>;
using int19 = nall::Integer<19>;
using int20 = nall::Integer<20>;
using int21 = nall::Integer<21>;
using int22 = nall::Integer<22>;
using int23 = nall::Integer<23>;
using int24 = nall::Integer<24>;
using int25 = nall::Integer<25>;
using int26 = nall::Integer<26>;
using int27 = nall::Integer<27>;
using int28 = nall::Integer<28>;
using int29 = nall::Integer<29>;
using int30 = nall::Integer<30>;
using int31 = nall::Integer<31>;
using int32 = nall::Integer<32>;
using int33 = nall::Integer<33>;
using int34 = nall::Integer<34>;
using int35 = nall::Integer<35>;
using int36 = nall::Integer<36>;
using int37 = nall::Integer<37>;
using int38 = nall::Integer<38>;
using int39 = nall::Integer<39>;
using int40 = nall::Integer<40>;
using int41 = nall::Integer<41>;
using int42 = nall::Integer<42>;
using int43 = nall::Integer<43>;
using int44 = nall::Integer<44>;
using int45 = nall::Integer<45>;
using int46 = nall::Integer<46>;
using int47 = nall::Integer<47>;
using int48 = nall::Integer<48>;
using int49 = nall::Integer<49>;
using int50 = nall::Integer<50>;
using int51 = nall::Integer<51>;
using int52 = nall::Integer<52>;
using int53 = nall::Integer<53>;
using int54 = nall::Integer<54>;
using int55 = nall::Integer<55>;
using int56 = nall::Integer<56>;
using int57 = nall::Integer<57>;
using int58 = nall::Integer<58>;
using int59 = nall::Integer<59>;
using int60 = nall::Integer<60>;
using int61 = nall::Integer<61>;
using int62 = nall::Integer<62>;
using int63 = nall::Integer<63>;
using int64 = nall::Integer<64>;

using  uint1 = nall::Natural< 1>;
using  uint2 = nall::Natural< 2>;
using  uint3 = nall::Natural< 3>;
using  uint4 = nall::Natural< 4>;
using  uint5 = nall::Natural< 5>;
using  uint6 = nall::Natural< 6>;
using  uint7 = nall::Natural< 7>;
using  uint8 = nall::Natural< 8>;
using  uint9 = nall::Natural< 9>;
using uint10 = nall::Natural<10>;
using uint11 = nall::Natural<11>;
using uint12 = nall::Natural<12>;
using uint13 = nall::Natural<13>;
using uint14 = nall::Natural<14>;
using uint15 = nall::Natural<15>;
using uint16 = nall::Natural<16>;
using uint17 = nall::Natural<17>;
using uint18 = nall::Natural<18>;
using uint19 = nall::Natural<19>;
using uint20 = nall::Natural<20>;
using uint21 = nall::Natural<21>;
using uint22 = nall::Natural<22>;
using uint23 = nall::Natural<23>;
using uint24 = nall::Natural<24>;
using uint25 = nall::Natural<25>;
using uint26 = nall::Natural<26>;
using uint27 = nall::Natural<27>;
using uint28 = nall::Natural<28>;
using uint29 = nall::Natural<29>;
using uint30 = nall::Natural<30>;
using uint31 = nall::Natural<31>;
using uint32 = nall::Natural<32>;
using uint33 = nall::Natural<33>;
using uint34 = nall::Natural<34>;
using uint35 = nall::Natural<35>;
using uint36 = nall::Natural<36>;
using uint37 = nall::Natural<37>;
using uint38 = nall::Natural<38>;
using uint39 = nall::Natural<39>;
using uint40 = nall::Natural<40>;
using uint41 = nall::Natural<41>;
using uint42 = nall::Natural<42>;
using uint43 = nall::Natural<43>;
using uint44 = nall::Natural<44>;
using uint45 = nall::Natural<45>;
using uint46 = nall::Natural<46>;
using uint47 = nall::Natural<47>;
using uint48 = nall::Natural<48>;
using uint49 = nall::Natural<49>;
using uint50 = nall::Natural<50>;
using uint51 = nall::Natural<51>;
using uint52 = nall::Natural<52>;
using uint53 = nall::Natural<53>;
using uint54 = nall::Natural<54>;
using uint55 = nall::Natural<55>;
using uint56 = nall::Natural<56>;
using uint57 = nall::Natural<57>;
using uint58 = nall::Natural<58>;
using uint59 = nall::Natural<59>;
using uint60 = nall::Natural<60>;
using uint61 = nall::Natural<61>;
using uint62 = nall::Natural<62>;
using uint63 = nall::Natural<63>;
using uint64 = nall::Natural<64>;