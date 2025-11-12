#pragma once
#include <charconv>
#include <immintrin.h>
#include <emmintrin.h>
#include <bit>

namespace number {

std::from_chars_result from_chars(const char *first, const char *last,
                                  unsigned &value, int base = 10) {

  static const __m128i zero = _mm_set1_epi8('0');
  static const __m128i nine = _mm_set1_epi8(9);
  static const uint64_t pows[] = {
    1000000000,
    100000000,
    10000000,
    1000000,
    100000,
    10000,
    1000,
    100,
    10,
    1,
    0,
    0,
    0
  };
  static const __mmask8 masks[] = {
    0x0,
    0x1,
    0x3,
    0x7,
    0xf,
    0xf,
    0xf,
    0xf,
    0xf,
    0xf,
    0xf,
    0xf
  };


  std::size_t n = last - first;
  if(n>10) {
    return {first,std::errc::result_out_of_range};
  }

  __m128i digits = _mm_lddqu_si128(reinterpret_cast<const __m128i_u*>(first));
  digits = _mm_sub_epi8(digits,zero);

  // validate input
  __mmask16 flags = _mm_cmpgt_epi8_mask(digits,nine);
  flags = _mm_cmpgt_epi8_mask(_mm_setzero_si128(),digits) | flags;
  int first_set = std::countr_zero(flags);
  if( first_set < n ) {
    return {first + first_set, std::errc::invalid_argument};
  }
  

  __m128i result = _mm_setzero_si128();
  __mmask8 overflow = 0;

  do {

    __m128i xdigits128 = _mm_cvtepu8_epi32(digits);
    __m256i xdigits = _mm256_cvtepu32_epi64(xdigits128);
  
    const __m256i_u* pow_ptr = reinterpret_cast<const __m256i_u*>(pows + 10 - n);
  
    __m256i factor = _mm256_lddqu_si256(pow_ptr);
    const __mmask8 mask = masks[n];
    

    __m256i values = _mm256_maskz_mul_epu32(mask, xdigits, factor);
    __m128i sum = _mm256_cvtepi64_epi32(values);
    __m128i old_result = result;
    result = _mm_add_epi32(sum,result);
    overflow = overflow | _mm_mask_cmp_epu32_mask(0xf,result,old_result,_MM_CMPINT_LT);
    if( n > 4) {
      n -= 4;
      digits = _mm_srli_si128(digits,4);
    } else {
      break;
    }
  } while(n>0);
  __m128i old_result = result;
  result = _mm_hadd_epi32(result,result);
  overflow = overflow | _mm_mask_cmp_epu32_mask(0x1,result,old_result,_MM_CMPINT_LT);
  old_result = result;
  result = _mm_hadd_epi32(result,result);
  overflow = overflow | _mm_mask_cmp_epu32_mask(0x1,result,old_result,_MM_CMPINT_LT);
  if(overflow) {
    return {first,std::errc::result_out_of_range};
  }

  value = _mm_cvtsi128_si32(result);
	   
    

  return {last,std::errc()};
  
}
  // a variant that does not use a power table, performance seems to be the same
std::from_chars_result from_chars1(const char *first, const char *last,
                                  unsigned &value, int base = 10) {

  static const __m128i zero = _mm_set1_epi8('0');
  static const __m128i reverse_mask = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
  static const __m256i init_factor = _mm256_set_epi64x(1000,100,10,1);
  static const __m256i factor_incr = _mm256_set1_epi64x(10000);
  
  static const __mmask8 masks[] = {
    0x0,
    0x1,
    0x3,
    0x7,
    0xf,
    0xf,
    0xf,
    0xf,
    0xf,
    0xf,
    0xf,
    0xf
  };

  std::size_t n = last - first;

  __m128i digits = _mm_lddqu_si128(reinterpret_cast<const __m128i_u*>(first - 16 + n));
  digits = _mm_sub_epi8(digits,zero);
  digits = _mm_shuffle_epi8(digits,reverse_mask);

  __m128i result = _mm_setzero_si128();
  __m256i factor = init_factor;

  do {

    __m128i xdigits128 = _mm_cvtepu8_epi32(digits);
    __m256i xdigits = _mm256_cvtepu32_epi64(xdigits128);
  
    const __mmask8 mask = masks[n];
    

    __m256i values = _mm256_maskz_mul_epu32(mask, xdigits, factor);
    __m128i sum = _mm256_cvtepi64_epi32(values);
    result = _mm_add_epi32(sum,result);
    if( n > 4) {
      n -= 4;
      digits = _mm_srli_si128(digits,4);
      factor = _mm256_mul_epu32(factor,factor_incr);
    } else {
      n = 0;
    }
  } while(n>0);

  result = _mm_hadd_epi32(result,result);
  result = _mm_hadd_epi32(result,result);

  value = _mm_cvtsi128_si32(result);
	   
  return {last,std::errc()};
  
}
  
} // namespace number
