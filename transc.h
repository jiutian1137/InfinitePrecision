#pragma once
/*{ "clmagic/calculation/fundamental/transcendental_functions.h":{
  "License": "Please identify Author",
  "Author": "LongJiangnan",
  "Date": "2019-2021",

  "decision-tree": {
  <!!! Only choose one, or make the same mistake>
    
    method1: { using template function,
      true: {
        (1) less code,
        (2) can still overrided,
        (3) Natural
      },
      false: {
        (1) difficult optimize, for example
              _Ty min(_Ty, _Ty)
              _Ty min(const _Ty&, const _Ty&)
              Which match ?
      }
    },

    method2: { using override function,
      true: {
        (1) more certain
      },
      false: {
        (1) more code,
        (2) same meaning has to implement many times
      }
    }
  }
} }*/


#include "float.h"
#include "rational.h"
//#include "complex.h"
//#include "quaternion.h"
#include <exception>
#include <numeric>
#include <vector>
namespace calculation 
{	

/* minimum value */
using std::min;
	
/* maximum value */
using std::max;

/* value clamp to [lower, upper] */
template<typename Ty1, typename Ty2> inline
Ty1 clamp(const Ty1& value, const Ty2 lower, const Ty2 upper) {
  if (value < lower) {
    return static_cast<Ty1>(lower);
  }
  
  if (value > upper) {
    return static_cast<Ty1>(upper);
  }

  return value;
}
	
/* value from [lower,upper] to [new_lower, new_upper] 
* upper-boundary(value == lower) = 0 + new_lower
* lower-boundary(value == upper) = 1 * (new_upper - new_lower) + new_lower
*/
template<typename Ty1, typename Ty2, typename Ty3> inline
Ty1 remap(const Ty1& value, const Ty2& lower, const Ty2& upper, const Ty3& new_lower, const Ty3& new_upper) {
  //assert(lower <= value && value <= upper);
  return static_cast<Ty1>((value - lower) / (upper - lower) * (new_upper - new_lower) + new_lower);
}

/* start to end by parameter(t) */
template<typename Ty1, typename Ty2> inline
Ty1 lerp(const Ty1& start, const Ty1& end, const Ty2& t) {
  return static_cast<Ty1>(start + (end - start) * t);
}
	
/* line(start0,start1) to line(end0,end1) by parameter(tX,tY) */
template<typename Ty1, typename Ty2> inline
Ty1 bilerp(const Ty1& start0, const Ty1& start1, const Ty1 end0, const Ty1 end1, const Ty2 tX, const Ty2 tY) {
  return static_cast<Ty1>(lerp( lerp(start0,start1,tX), lerp(end0, end1,tX), tY ));
}
	
/* plane(start00,start10,start01,start11) to plane(end00,end10,end01,end11) by parameter(tX,tY,tZ) */
template<typename Ty1, typename Ty2> inline
Ty1 trilerp(const Ty1& start00, const Ty1& start10, const Ty1& start01, const Ty1 start11, const Ty1& end00, const Ty1& end10, const Ty1& end01, const Ty1 end11, 
  const Ty2 tX, const Ty2 tY, const Ty2 tZ) {
  return static_cast<Ty1>(lerp( bilerp(start00,start10,start01,start11,tX,tY), bilerp(end00,end10,end01,end11,tX,tY), tZ ));
}
	
/* bilinear sample function(f) at (u,v) */
template<typename Fn, typename Ty> requires requires(Fn __f, Ty __t) { __f(__t, __t); }
auto bilersmp(const Fn& latticef, Ty u, Ty v) {
  // setup lerp parameters
  size_t x = static_cast<size_t>(floor(u));
  size_t y = static_cast<size_t>(floor(v));
  Ty tX = fract(u);
  Ty tY = fract(v);
			
  // lerp four lattices
  auto sample = latticef(x,y);
  if ( tX != 0 ) {
    sample = lerp(sample, latticef(x+1,y), tX);
    if ( tY != 0 ) {
      sample = lerp(sample, lerp(latticef(x,y+1),latticef(x+1,y+1),tX), tY);
    }
  } else {
    if ( tY != 0 ) {
      sample = lerp(sample, latticef(x,y+1), tY);
    }
  }

  return sample;
}
	
/* trilinear sample function(f) at (u,v,w) */
template<typename Fn, typename Ty> requires requires(Fn __f, Ty __t) { __f(__t, __t, __t); }
auto trilersmp(const Fn& latticef, Ty u, Ty v, Ty w) {
  // setup lerp parameters
  size_t x = static_cast<size_t>(floor(u));
  size_t y = static_cast<size_t>(floor(v));
  size_t z = static_cast<size_t>(floor(w));
  Ty tX = fract(u);
  Ty tY = fract(v);
  Ty tZ = fract(w);
			
  // lerp eight lattices
  auto sample = latticef(x,y,z);
  if ( tX != 0 ) {
    size_t x1 = x + 1;
    sample = lerp(sample, latticef(x1,y,z), tX);
    if ( tY != 0 ) {
      size_t y1 = y + 1;
      sample = lerp(sample, lerp(latticef(x,y1,z),latticef(x1,y1,z),tX), tY);
      if ( tZ != 0 ) {
        size_t z1 = z + 1;
        sample = lerp(sample, lerp(lerp(latticef(x,y,z1),latticef(x1,y,z1),tX),lerp(latticef(x,y1,z1),latticef(x1,y1,z1),tX),tY), tZ);
      }
    } else {
      if ( tZ != 0 ) {
        // lerp XZ
        size_t z1 = z + 1;
        sample = lerp(sample, lerp(latticef(x,y,z1),latticef(x1,y,z1),tX), tZ);
      }
    }
  } else {

    if ( tY != 0 ) {
      size_t y1 = y + 1;
      sample = lerp(sample, latticef(x,y1,z), tY);
      if ( tZ != 0 ) {
        size_t z1 = z + 1;
        sample = lerp(sample, lerp(latticef(x,y,z1),latticef(x,y1,z1),tY), tZ);
      }
    } else {
      if (tZ != 0) {
        sample = lerp(sample, latticef(x,y,z+1), tZ);
      }
    }

  }

  return sample;
}

/* greatest common divisor */
using std::gcd;

/* least commmon multiplier */
using std::lcm;


#ifndef __CLMAGIC_CALULATION_SUM
#define __CLMAGIC_CALULATION_SUM
/* sum<i=start, end>( f(i) ) */
template<typename Integer, typename Function>
auto sum(Integer start, Integer end, Function f) -> decltype( f(start)+f(end) ) {
  auto result = f( start );
  for (Integer i = start+1; i <= end; ++i) {
    result += f( i );
  }

  return std::move( result );
}
#endif

/* product<i=start, end>( f(i) ) */
template<typename Integer, typename Function>
auto product(Integer start, Integer end, Function f) -> decltype( f(start)*f(end) ){
  auto result = f( start );
  for (Integer i = start+1; i <= end; ++i) {
    result *= f( i );
  }

  return std::move( result );
}

/**
 * factorial:
 *   1*2*3*...*order,
 *   zeroe-order is 1 .
 * 
 * @param order 
 *   is integer
 * 
 * @symbol
 *   fact(order) = order!
 * 
 * @formula
 *   fact(order) = 1*2*3*...*order
*/
template<typename Number = unsigned long long>
Number fact(Number order) {
  assert( order >= 0 );
  assert( floor(order) == order );
  Number result = 1;
  for (size_t i = 2/*ignore'1'*/; i <= static_cast<size_t>(order); ++i) {
    result *= i;
  }

  return result;
}

/**
 * shifted_factorial:
 *   shift*(shift + 1)*(shift + 2)*(shift + 3)*...*(shift+order-1),
 *   zeroe-order is 1 .
 * 
 * @param order 
 *   is any number, must (shift + integer)
 * 
 * @param shift 
 *   is any number
 * 
 * @symbol
 *   fact(order,shift) = <shift>order
 * 
 * @formula
 *   fact(order,shift) = shift*(shift + 1)*(shift + 2)*(shift + 3)*...*(shift+order-1)
 *   fact(order,1) = fact(order)
 * 
 * @relation for 'gamma(x)'
 *                        gamma(shift+order)
 *   fact(order,shift) = --------------------
 *                        gamma(shift) 
*/
template<typename Number = unsigned long long>
Number fact(Number order, Number shift) {
  assert( order >= 0 );
  assert( floor(order) == order );
  Number result = 1;
  for (size_t i = 0; i != static_cast<size_t>(order); ++i) {
    result *= (i + shift);
  }

  return result;
}

/**
 * double factorial: 
 *   zero-order: 1, 
 *   odd-order: 1*3*5*7*...*(order-2)*order, 
 *   even-order: 2*4*6*8*...*(order-2)*order.
 * 
 * @param order 
 *   is integer
 * 
 * @relation for 'fact' 
 *   fact2(order) * fact2(order-1) = fact(order)
 * 
 * @relation for 'gamma'
 *    fact2(2*order - 1)
 *   -------------------- = gamma(order+0.5)
 *    2^order * sqrt(pi)
*/
template<typename Number = unsigned long long>
Number fact2(Number order) {
  assert( order >= 0 );
  assert( floor(order) == order );
  if constexpr (std::is_same_v<Number, unsigned int>) {
    if( order > 20 ) { throw std::overflow_error("fact2(order)"); }
  } else if constexpr (std::is_same_v<Number, unsigned long long>) {
    if( order > 33 ) { throw std::overflow_error("fact2(order)"); }
  }
  bool   odd    = static_cast<size_t>(order) & 1;
  Number result = 1;
  for (size_t i = odd?1:2; i <= static_cast<size_t>(order); i+=2) {
    result *= i;
  }

  return result;
}

/**
 * binomial coefficient:
 *   pow(a + b, 2) = binomial(2,0)*a*a + binomial(2,1)*a*b + binomial(2,2)*b*b .
 * 
 * @param power 
 *   is integer
 * 
 * @param nth 
 *   is integer
 * 
 * @formula
 *                                  1*2*3*...*power                 power!
 *   binomial(power,nth) = ------------------------------- = ---------------------
 *                          1*2*3*nth * 1*2*3*(power-nth)     nth! * (power-nth)!
 *
 * @formula shift 'nth'
 *                            (nth+1)*(nth+2)*...*power        <nth+1>(power-nth)
 *   binomial(power,nth) = -------------------------------- = --------------------
 *                                      1*2*3*(power-nth)        (power-nth)!
 * 
 * @formula shift 'power-nth'
 *                          (power-nth+1)*(power-nth+2)*...*power      <power-nth+1>(nth)
 *   binomial(power,nth) = ---------------------------------------  = --------------------
 *                          1*2*3*nth                                         nth!
 * 
 * @binomial theorem (power is positive integer)
 *          power                                            power-nth    nth
 *   (a + b)^     = sum<k=nth,power>( binomial(power,nth) * a^         * b^   )
 * 
 * @binomial series (power is any number)
 *          power                    power*(power-1)*...*(power-nth+1)     nth
 *   (1 + x)^     = sum<nth=0,inf>( ----------------------------------- * x^   )
 *                                               1*2*3*nth
 * 
 *                                   gamma(power+1)/gamma(power-th+1)     nth
 *                = sum<nth=0,inf>( ---------------------------------- * x^   )
 *                                                nth!
*/
template<typename Integer = unsigned long long>
Integer binomial(size_t power, size_t nth) {
#if 1

  Integer result_num = 1;
  Integer result_den = 1;

  assert( nth <= power );
  if ( nth > power - nth ) {
    Integer k = nth + 1;
    Integer n = power - nth; 
    for ( ; k <= power; ++k, --n) {
      result_num *= k;
      result_den *= n;
      Integer divsor = gcd(result_num, result_den);
      result_num /= divsor;
      result_den /= divsor;
    }
  } else {
    Integer k = power - nth + 1;
    Integer n = nth;
    for ( ; k <= power; ++k, --n) {
      result_num *= k;
      result_den *= n;
      Integer divsor = gcd(result_num, result_den);
      result_num /= divsor;
      result_den /= divsor;
    }
  }

  assert( result_num % result_den == 0 );
  return result_num / result_den;

#else

  // C(n,0) = C(n,n) = 1
  if (nth == 0 || nth == power) {
    return 1;
  }

  // C(n,1) = C(n,n-1) = n
  if (nth == 1 || nth + 1 == power) {
    return power;
  }

  /**
   * 1
   * 1 1
   * ....
   * ......
   *       .. <----- k
   * 1      ... cache[nth]    { 1 }      :for n == power-2
   *             \     |   \    |
   *              \    |    \   |
   *               \   |     \  |
   *       .. <----- k-1 <----- k
   * 1      ... cache[nth-1] cache[nth]  :for n == power-1
   *                      \     |
   *                       \    |
   *                        \   |
   *                         (p,n)       :return
  */
  std::vector<Integer> cache;
  cache.resize(nth + 1, 1);
  for (size_t n = 2; n != power; ++n) {
    // clip right{ ignore_terms }
    size_t k = min(size_t(n), nth/*cache.size()-1*/);
    // clip right{ C(n,n) }
    if (k == size_t(n)) {
      --k;
    }
    // clip left{ ignore_terms }
    size_t final_k = nth/*cache.size()-1*/ - min(power-n, nth);
    // clip left{ C(n,0) }
    size_t last_k = final_k == 0 ? 0 : final_k - 1;
						
    for ( ; k != last_k; --k) {
      cache[k] += cache[k - 1];
    }
  }

  return cache[nth] + cache[nth - 1];

#endif
}

/**
 * bernoulli numbers: 
 *   power series 
 * 
 * @generating function
 *        x                        B[k]*pow(x,k)
 *   ------------ = Sum[k=0,inf]( --------------- )
 *    exp(x) - 1                        k!
 * 
 * @calculate formula
 *                         1                    r                n
 *   B[n] = Sum[k=0,n]( -------*Sum[r=0,k]( (-1)^*binomial(k,r)*r^ ) )
 *                       k + 1
 *
 * @relation for recurrence 
 * 
 *    Sum[k=0,n-1]( B[k]*binomial(n,k) ) == 0
 * 
 *    Sum[k=0,n-1]( B[k]*binomial(n+1,k) )
 *   -------------------------------------- == B[n]
 *             - binomial(n+1,n)
 * 
 * ...
*/
template<typename Rational/* = rational64_t*/>
std::vector<Rational> bernoulli_numbers(size_t index) {
  auto number_array = std::vector<Rational>(index + 1); 
       number_array[0] = 1;
  for (size_t n = 1; n <= index; ++n) {
    number_array[n] = - sum(size_t(0), n-1, [n,&number_array](size_t k){ return number_array[k] * binomial(n+1, k); })
                        / binomial(n+1, n);
  }

  return std::move(number_array);
}


/**
 * exponential function
 *   for 'integer' power
 * @param x 
 *   (-inf, inf)
 * @param power 
 *   any integer
 * @return 
 *   (-inf, inf)
*/
template<typename Number>
Number pow(const Number& x, int power) {
  if ( power < 0 ) {
    return pow(1/x, abs(power));
  }

  /** I write at 2021.5.20, LongJiangnan
   * 1100 1011 = 203
   *
   * =  2^7 * 1
   *  + 2^6 * 1
   *  + 2^5 * 0
   *  + 2^4 * 0
   *  + 2^3 * 1
   *  + 2^2 * 0
   *  + 2^1 * 1
   *  + 2^0 * 1
   * 
   * =  2^7 * (203 - 2^0 - 2^1 - 2^3 - 2^6)%256/128
   *  + 2^6 * (203 - 2^0 - 2^1 - 2^3)%128/64
   *  + 2^5 * (203 - 2^0 - 2^1 - 2^3)%64/32
   *  + 2^4 * (203 - 2^0 - 2^1 - 2^3)%32/16
   *  + 2^3 * (203 - 2^0 - 2^1)%16/8
   *  + 2^2 * (203 - 2^0 - 2^1)%8/4
   *  + 2^1 * (203 - 2^0)%4/2
   *  + 2^0 * 203%2/1
   *                i                                  i+1    i
   * = sum<i=0,7>( 2^ * ( (203 - sum<k=0,i-1>(...)) % 2^   / 2^ ) )
   * 
   * 
   * pow(x,203)
   *           7    6    3    1    0
   * = pow(x, 2^ + 2^ + 2^ + 2^ + 2^)
   *          7           6           3           1           0
   * = pow(x,2^) * pow(x,2^) * pow(x,2^) * pow(x,2^) * pow(x,2^)
   *                           i                                  i+1    i
   * = product<i=0,7>( pow(x, 2^ * ( (203 - sum<k=0,i-1>(...)) % 2^   / 2^ )) )
   *                           i                                        i+1    i
   * = product<i=0,7>( pow(x, 2^) * pow(x, (203 - sum<k=0,i-1>(...)) % 2^   / 2^) )
  */
  int seriesP = 0;
  int termP = 1;
  Number series = 1;
  Number term = x;
  for (int i = 0; seriesP != power; ) {
    if ( (power - seriesP) % (termP<<1) ) {
      seriesP += termP;
      series *= term;
    }

    termP <<= 1;
    term *= term;
  }

  return series;
}

/**
 * @brief 
 *   square root
 * @param x 
 *   [0,inf)
 * @return 
 *   [0,inf)
 * @see
 *   quadratic(a,b,c,roots)
*/
template<typename Number>
Number sqrt(const Number& x) {
  if ( x > 0 ) {

	  const Number eps = pow(static_cast<Number>(2), - static_cast<int>(std::numeric_limits<Number>::digits/2 + 2));//@see cbrt(x)
    Number y = x;
    Number ym1;
    do {
      ym1 = y;
      y = (y + x/y) / 2;// y_next = y - (y-x/y)/2
    } while ( abs(y-ym1) >= eps*abs(y) );

    return y;
		
  } else if ( x == 0 ) {

    return static_cast<Number>(0);

  } else   /* x <  0 */{

    return std::numeric_limits<Number>::quiet_NaN();

  }
}

/**
 * @brief 
 *   cubic root
 * @param x 
 *   (-inf, inf)
 * @return 
 *   (-inf, inf)
 * @see
 *   cubic(a,b,c,d,roots)
*/
template<typename Number>
Number cbrt(const Number& x) {
  if (x == 0) {
    // not convergence
    return static_cast<Number>(0);
  }

  /** book: <Numerical Analysis> Timothy Sauer
   * 
   * pow(2,-1): 0.5   ~= convergence_error
   * pow(2,-1*3): 0.125   ~= next convergence_error
   * pow(2,-1*3*3): 0.001953125   ~= next next convergence_error
   * pow(2,-1*3*3*3): 0.000000007450580596923828125
   * ...
   * pow(2,-digits/3): ....
   * pow(2,-digits): is 'epsilon'
   * 
   * if convergence start, error must pass (0.X|0.0X|...),
   * then error alone the formula 'pow(error,3)' go to end...
   * so we get prev error of 'epsilon', can avoid round-loop at 'epsilon', notice we added a tolerance
  */
  const Number eps = pow(static_cast<Number>(2), - static_cast<int>(std::numeric_limits<Number>::digits/3 + 2));
  Number y = x;
  Number factor;
  do {
    Number yyy = y * y * y;
    factor = (yyy + x + x) / (yyy + yyy + x);
    y *= factor;
  } while ( abs(1 - factor) >= eps );

  return y;
}

/**
 * exponential function 
 *   for e(2.71828...)
 * 
 * @param x
 *   (-inf, inf)
 * 
 * @return
 *   (0, inf)
 * 
 * @law
 *   exp(a)*exp(b) = exp(a+b)
 * 
 *   exp(a)/exp(b) = exp(a-b)
 * 
 *   exp(-x) = 1/exp(x)
 * 
 * @derivative
 *   differentiate(exp,x) = exp(x)
 * 
 * #integral
 *   integrate(exp(x),dx) = exp(x) + constant
 * 
 * @alternative
 *   exp(x) = limit<n->inf>( pow(1 + x/n, n) )
 * 
 *   exp(x) = sum<k=0,inf>( pow(x,k) / fact(k) )
 * 
 *   ...
*/
template<typename Number>
Number exp(const Number& x) {
  if ( x < 0 ) {
    // convergence slow, odd terms are negative, even terms are positive 
    return 1 / exp(abs(x));
  } else if ( x > 1 ) {
    // convergence slow, see @alternative for 'maclaurin_series'
    Number xi = floor(x);
    return exp(x - xi) * pow(exp(static_cast<Number>(1)), static_cast<int>(xi));
  } else if ( x == 0 ) {
    // seriesR not convergence
    return static_cast<Number>(1);
  }

  assert( 0 < x && x <= 1 );
  // sum maclaurin_series
  Number series = 0;
  const Number eps = static_cast<Number>(0.01);
  Number seriesR = 0;
  const Number epsR = std::numeric_limits<Number>::epsilon();
  
  Number n = 0;
  Number term = 1;
  do {
    series += term;
    /**
     *  nth[n+1]      pow(x,n+1)     pow(x,n)      x
     * ----------- = ------------ / ---------- = -----
     *  nth[n]        fact(n+1)      fact(n)      n+1
    */
    term *= ( x/(n += 1) );
  } while ( term >= eps*series );
  do {
    seriesR += term;
    term *= ( x/(n += 1) );
  } while ( term >= epsR*seriesR );

  return series + seriesR;
/** exponential function details
 * 
 * @alternative for 'limit'
 *   exp(x)
 *                          x  n
 *    = limit<n->inf>( 1 + ---)^ )
 *                          n
 * 
 * @alternative for 'maclaurin_series'
 *   exp(x)
 *                     @(X=0) dexp(X)     k
 *    = sum<k=0,inf>( ---------------- * x^ )       :1. integration by parts
 *                           k!
 *                     exp(0)     k
 *    = sum<k=0,inf>( -------- * x^ )               :   dexp(X) = exp(X)
 *                       k!
 *                      1      k
 *    = sum<k=0,inf>( ----- * x^ )
 *                      k!
*/
}

/**
 * inverse hyperbolic tangent
 *
 * @param x 
 *   (-1, 1)
 * 
 * @return 
 *   (-inf, inf)
 * 
 * @addition formula
 *   atanh(u) + atanh(v) == atanh((u+v) / (1 + u*v))
 * 
 * @derivative
 *   differentiate( atanh, x ) = 1/(1 - x*x)
 * 
 * @integral
 *   integrate( atanh(x), dx ) = log(1 - x*x)/2 + x*atanh(x) + constant
 * 
 * @alternative
 *   atanh(x) = sum<k=0,inf>( pow(x,2*k+1)/(2*k+1) )    :maclaurin_series
 * 
 *   atanh(x) = asinh( x/sqrt(1-x*x) ) = (+ -)acosh( 1/sqrt(1-x*x) )
 * 
 *   atanh(x) = log((1+x)/(1-x))/2
*/
template<typename Number>
Number atanh(const Number& x) {
  assert( -1 < x && x < 1 );
  if ( x < 0 ) {
    // symmetry around X-axis
    return -atanh(x);
  } else if ( x == 0 ) {
    // not convergence
    return static_cast<Number>(0);
  }

  assert( 0 <= x && x < 1 );
  // sum maclaurin_series
  Number series = 0;
  const Number eps = static_cast<Number>(0.01);
  Number seriesR = 0;
  const Number epsR = std::numeric_limits<Number>::epsilon();

  const Number x_x = x * x;
  Number term = x;
  Number k = 0;
  do {
    series += term;
    /** 
     *  term[k+1]     x^(2*(k+1)+1)     x^(2*k+1)
     * ----------- = --------------- / -----------
     *  term[k]         2*(k+1)+1         2*k+1
     * 
     *                x^(2*k+3)     2*k+1
     *             = ----------- * -------
     *                x^(2*k+1)     2*k+3
     * 
     *             = x*x * (2*k+1)/(2*k+3)
    */
    term *= x_x * (2*k+1)/(2*k+3);
    k += 1;
  } while ( term >= eps*series );
  do {
    seriesR += term;
    term *= x_x * (2*k+1)/(2*k+3);
    k += 1;
  } while ( term >= epsR*seriesR );

  return series + seriesR;
/** inverse hyperbolic tangent details
 * 
 * @addition formula
 *
 *   atanh(u) + atanh(v)
 * 
 *    = ...
 * 
 *              u + v
 *    = atanh(---------)
 *             1 + u*v
 *
 * @derivative
 *                                  
 *   differentiate( atanh, x )
 *  
 *    = ...
 *
 *          1
 *    = ---------
 *       1 - x*x
 * 
 * @alternative for 'maclaurin_series'
 *
 *   atanh(x)
 *                         1
 *    = integral<0,x>( ---------, dx )
 *                      1 - x*x
 *                              -1
 *    = integral<0,x>( (1 + -x*x)^ , dx )                                  :1. binomial_series expansion
 *
 *                                                          k
 *    = integral<0,x>( sum<k=0,inf>( binomial(-1,k) * (-x*x)^ ), dx )
 * 
 *                                                        k    2*k
 *    = integral<0,x>( sum<k=0,inf>( binomial(-1,k) * (-1)^ * x^   ), dx )
 *
 *                                         k                  2*k
 *    = sum<k=0,inf>( binomial(-1,k) * (-1)^ * integral<0,x>(x^  , dx) )   :2. exchange sum<> and integral<>, and integrate
 *
 *                                         k    x^(2*k+1)
 *    = sum<k=0,inf>( binomial(-1,k) * (-1)^ * ----------- )
 *                                                        2*k+1
 *                     -1*-2*-3*...*(-1-k+1)        k    x^(2*k+1)
 *    = sum<k=0,inf>( ----------------------- * (-1)^ * ----------- )      :3. simplify
 *                              k!                         2*k+1
 *                     x^(2*k+1)
 *    = sum<k=0,inf>( ----------- )                                        :   -1^k * -1*-2*-3*...*(-1-k+1) = k!
 *                       2*k+1
 * 
 * @alternative for 'asinh'
 *
 *   atanh(x)
 * 
 *    = ...
 * 
 *                  x
 *    = asinh(-------------)
 *             sqrt(1-x*x)
 * 
 * @alternative for 'acosh'
 *
 *   atanh(x)
 *
 *    = ...
 * 
 *                       1
 *    = (+ -)acosh(-------------)
 *                  sqrt(1-x*x)
 *
 * 
 * @link 
 *   "https://mathworld.wolfram.com/InverseHyperbolicTangent.html"
 *   "https://en.wikipedia.org/wiki/Inverse_hyperbolic_functions"
*/
}

/**
 * natural logarithm function:
 *   logarithm for e(2.7182...)
 *
 * @param x_significand 
 *   (0, inf)
 * 
 * @return 
 *   (-inf, inf)
 * 
 * @law
 *   log(a*b) = log(a) + log(b)
 * 
 *   log(a/b) = log(a) - log(b)
 * 
 *   log(x^b) = log(x) * b
 * 
 * @derivative
 *   differentiate(log,x) = 1/x
 * 
 * @integral
 *   integrate(log(x),dx) = x*(log(x)-1) + constant
 * 
 * @alternative
 *   log(x) = log( x.significand() * pow(2,x.exponent()) ) = log( x,significand() ) + x.exponent()*log(2)
 *
 *   log(x) = atanh((x - 1)/(x + 1)) * 2
 * 
 *   ...
*/
template<typename Number> inline
Number log(const Number& x_significand, const Number& x_exponent2) {
  if ( x_significand == 0 ) {
    return - std::numeric_limits<Number>::infinity();
  }
  return atanh((x_significand - 1)/(x_significand + 1)) * 2
    + x_exponent2 * atanh(static_cast<Number>(2-1)/(2+1)) * 2;
}

/**
 * natural logarithm function:
 *   logarithm for e(2.7182...)
 * @param x 
 *   [0,inf)
 * @return 
 *   (-inf,inf)
*/
template<size_t s, size_t e, bool opt> inline
floatX<s,e,opt> log(const floatX<s,e,opt>& x) {
  return log(x.significand(), x.exponent());
}

/**
 * exponential function:
 *   y = x^power
 * 
 * @param x 
 *   [0,inf) | (power is integer)(-inf,inf)
 * 
 * @param power 
 *   is real number
 * 
 * @alternative
 *    power      log(x)  power    log(x) * power
 *   x^     = ( e^      )^     = e^
*/
template<typename Number> inline
Number pow(const Number& x, const Number& power) {
  if ( floor(power) == power ) {
    return pow(x, static_cast<int>(power));
  }
  return exp(log(x) * power);
}

/**
 * inverse hyperbolic sine
 * 
 * @param x 
 *   (-inf, inf)
 * 
 * @return 
 *   (-inf, inf)
 * 
 * @derivative
 *                                     1
 *   differentiate( asinh, x ) = ---------------
 *                                sqrt(x*x + 1)
 * 
 * @integration
 *   integrate( asinh(x), dx ) = x*asinh(x) - sqrt(x*x + 1) + C
 * 
 * @alternative
 *   asinh(x) = log( sqrt(x*x + 1) + x )
 * 
 *                                    1
 *   asinh(X) = integrate<0,X>( ---------------, dx )
 *                               sqrt(x*x + 1)
 * 
 *                             0.5*1.5*...*(0.5+k-1)            x^(2*k+1)
 *   asinh(x) = sum<k=0,inf>( ----------------------- * -1^k * ----------- )
 *                                       k!                       2*k+1
 * 
 * ...
 * 
 * @link
 *   "https://www.wolframalpha.com/input/?i=asinh%28x%29"
 *   "https://en.wikipedia.org/wiki/Inverse_hyperbolic_functions"
 *   "https://mathworld.wolfram.com/InverseHyperbolicFunctions.html"
*/
template<typename Number> inline
Number asinh(const Number& x) {
  return log(sqrt(x*x + 1) + x);
}

/**
 * inverse hyperbolic cosine
 * 
 * @param x 
 *   [1, inf)
 * 
 * @return 
 *   [0, inf)
 * 
 * @derivative
 *                                        1
 *   differentiate( acosh, x ) = ---------------------
 *                                sqrt(x-1)*sqrt(x+1)
 * 
 * @integration
 *   integrate( acosh(x), dx ) = x*acosh(x) - sqrt(x-1)*sqrt(x+1) + C
 * 
 * @alternative
 *   acosh(x) = ....   :maclaurin_series
 * 
 *   acosh(x) = log(x + sqrt(x-1)*sqrt(x+1))
 * 
 *               acosh(2*x^2 - 1)     acosh(8*x^4 - 8*x^2 + 1)
 *   acosh(x) = ------------------ = --------------------------
 *                       2                       4
*/
template<typename Number> inline
Number acosh(const Number& x) {
  return log(x + sqrt(x-1)*sqrt(x+1));
}


/**
 * calculate pi:
 *   use Bailey-Borwein-Plouffe formula
 *
 * @return 
 *   pi: ratio of the circumference of a circle to its diameter
 * 
 * @Bailey-Borwein-Plouffe formula for 'pi'
 *   derivation part ...
 *                        1          4           2           1           1
 *   pi = sum<k=0,inf>( ------*( --------- - --------- - --------- - --------- ) )
 *                       16^k     8*k + 1     8*k + 4     8*k + 5     8*k + 6
*/
template<typename Number>
Number calculate_pi() {
  Number series = 0;
  const Number eps = static_cast<Number>(0.01);
  Number seriesR = 0;
  const Number epsR = std::numeric_limits<Number>::epsilon();

  size_t k = 0;
  Number k8 = 0;
  Number a = 1;
  Number term = a * (4/(k8 + 1) - 2/(k8 + 4) - 1/(k8 + 5) - 1/(k8 + 6));
  do {
    ++k;
		series += term;
    a /= 16;
    k8 += 8;
    term = a * (4/(k8 + 1) - 2/(k8 + 4) - 1/(k8 + 5) - 1/(k8 + 6));
  } while ( abs(term) >= eps * abs(series) );
  do {
    ++k;
    seriesR += term;;
    a /= 16;
    k8 += 8;
    term = a * (4/(k8 + 1) - 2/(k8 + 4) - 1/(k8 + 5) - 1/(k8 + 6));
  } while ( abs(term) >= epsR * abs(seriesR) );

	return series + seriesR;
}

template<typename Number, typename Rational>
Number fmod_pi(const Number& x, const Rational pi_scale, const Number pi = static_cast<Number>(3.141592653589793)) {
  /** 
   * @the problem
   *  pi's precision is finite, for example 16digit,
   *  so, arithmetic only decrease precision, then less equal than 16digit,
   *  decrease precision at binary 'substraction' with two almost equal numbers, final less than 16digit.
   * 
   * @variables
   *   input number is finite precision, this is correct,
   *   but pi should infinite precision.
   * 
   * @solusion
   *   we replace 'pi' by Bailey-Borwein-Plouffe-series, 
   *   but still accuracy not change or more bad.
   * 
   *   @analysis
   *     x - trunc(x/pi)*pi = y, x = 10000
   * 
   *       10000.0                 :16 precision
   *     -  9973.399999999999'8939 :16 precision, 3183 * 3.1333333333333333 = 9973.4
   *     =    26.600000000000'364  :14 precision, see the problem line3
   * 
   *       26.600000000000'36 4     :14 precision
   *     - 25.747710622710 62'51517 :16 precision, 3183 * 0.0080891330891330899 = 25.747710622710622710622710622711
   *     =  0.852289377289'73 975   :12 precision, see the problem line3
   * 
   *       0.852289377289'7397 5    :12 precision
   *     - 0.524952850458 3650'8248 :16 precision, 3183 * 0.00016492392411510056 = ...
   *     = 0.327336526831'3747 1    :12 precision
   * 
   *     ......
   *     ...
   * 
   *     'first few' terms in series of 'pi', that decrease precision, 
   *     because this target is 'modulo', no matter how large number, that must substracted by 'almost equal' numbers in process,
   *     'after' terms are steady...
   *     
   *  @improvement
   *    arithmetic for 'first few' terms, we use 'rational number' calculate, since 'pi' is natural.
   *    the first, we split 'x' to integer part and fraction part.
   * 
   *    x = i + f
   *    n = trunc(x / pi)
   *    y = x - pi*n
   *
   *    y/n = x/n - pi
   *    y/n = (i + f)/n - pi
   *    y/n = i/n + f/n - pi
   *    y/n = (i/n - pi) + f/n
   *    y = (i/n - pi)*n + f
   * 
   *    we get almost all precision, input x less than 1000000.0 .
   *    !Notice: except when 'y' near to 0 !!!!!!!!!!!!!!!!!!!!!!!!!
  */

  long long n = static_cast<long long>( trunc(x / (pi*static_cast<Number>(pi_scale))) );
  if ( n == 0 ) {
    return x;
  }
  if ( abs( frac(x / (pi*static_cast<Number>(pi_scale)))) < std::numeric_limits<Number>::epsilon() ) {
    return 0;
  }
  
  Rational xi = Rational(static_cast<long long>(trunc(x)), n);
  int k8i = 0;
  long long exp16_ki = 1;
  try {
	  do {
      xi = xi - Rational(4,k8i+1)/exp16_ki*pi_scale
        + Rational(2,k8i+4)/exp16_ki*pi_scale
        + Rational(1,k8i+5)/exp16_ki*pi_scale
        + Rational(1,k8i+6)/exp16_ki*pi_scale;
      if ( exp16_ki > std::numeric_limits<long long>::max() / 16 ) {
        break;
      }
      k8i += 8;
      exp16_ki *= 16;
    } while ( true );
  }
  catch (const std::underflow_error&) {
    // ...
  }
  catch (const std::overflow_error&) {
    // ...
  }

  const Number eps = std::numeric_limits<Number>::epsilon();
  Number yi = static_cast<Number>(xi);
  Number k8 = static_cast<Number>(k8i);
  Number exp16_k = static_cast<Number>(exp16_ki);
  Number term = ( 4/(k8 + 1) - 2/(k8 + 4) - 1/(k8 + 5) - 1/(k8 + 6) ) / exp16_k;
	do {
    yi -= term*static_cast<Number>(pi_scale);
    k8 += 8;
    exp16_k *= 16;
		term = ( 4/(k8 + 1) - 2/(k8 + 4) - 1/(k8 + 5) - 1/(k8 + 6) ) / exp16_k;
	} while ( abs(term) >= eps * abs(yi) );

  return yi*n + frac(x);
}

template<typename Number>
Number cos(const Number& x, const Number pi);

/**
 * sine function:
 *   use maclaurin_series
 * 
 * @param x
 *   [-pi*2, pi*2], 
 *   inaccurate in (-inf, inf) because substraction in fmod(x, pi/2), 
 *   inaccurate at slightly great than pi/2 because ... in fmod(x, pi/2).
 * 
 * @return 
 *   [0, 1]
 * 
 * @derivative
 *   differentiate(sin, x) = cos(x)
 * 
 * @integral
 *   integrate(sin(x), dx) = -cos(x) + constant
 * 
 * @series expansion
 *   sin(x) = -sin(x)              :odd function
 * 
 *   sin(x) = sin(x + pi*2 * n)    :period function
 * 
 *   sin(x*2) = 2 * sin(x) * cos(x)             :double angle law
 * 
 *   sin(x/2) = sqrt((1 - cos(x))/2)            :half angle law
 * 
 *   sin(a+b) = sin(a)*cos(b) + cos(a)*sin(b)   :sum law
 *  
 *   sin(a-b) = sin(a)*cos(b) - cos(a)*sin(b)   :difference law
 * 
 *   sin(a) + sin(b) = 2 * sin((a+b)/2) * cos((a-b)/2)   :sum to product law
 * 
 *   sin(a) * sin(b) = 1/2 * ( cos(a-b) - cos(a+b) )     :product to sum law
 * 
 *   sin(x) = sum<k=0,inf>( pow(-1,k)/fact(2*k+1) * pow(x,2*k+1) )   :maclaurin_series, only odd term, odd is 2*k+1
*/
template<typename Number>
Number sin(const Number& x, const Number pi = static_cast<Number>(3.141592653589793)) {
  if ( x < 0 ) {
    //  convergence optimize, sin(x) = -sin(-x)
    return -sin(abs(x), pi);
  } else if ( x == 0 ) {
    // not convergence
    return static_cast<Number>(0);
  } else if ( x > pi/2 ) {
    // convergence optimize
    const Number pio2 = pi / 2;
#if defined __has_include && __has_include("rational.h")
    Number y = fmod_pi(x, rational64_t(1,2), pi);
#else 
    Number y = fmod(x, pio2);
#endif
    switch ( static_cast<int>(trunc(x/pio2)) % 4 ) {
      case 0: return sin(y, pi);// 0->1
      case 1: return cos(y, pi);// 1->0
      case 2: return -sin(y, pi);// 0->-1
      case 3: return -cos(y, pi);// -1->0
    }
  }

  assert( abs(x) <= pi/2 );
  // sum maclaurin_series
  const Number eps = std::numeric_limits<Number>::epsilon();
  Number series = 0;
  const Number epsR = static_cast<Number>(0.01);
  Number seriesR = 0;

  const Number neg_x_x = -(x*x);
  Number term = x;
  Number k = 0;
  do {
    series += term;
    /**
     *  term[k+1]       -1^(k+1)       2*(k+1)+1      -1^k       2*k+1
     * ----------- = -------------- * x^         / ---------- / x^
     *  term[k]       (2*(k+1)+1)!                  (2*k+1)!
     * 
     *                     (2*k+1)!     x^(2*k+3)
     *             = -1 * ---------- * -----------
     *                     (2*k+3)!     x^(2*k+1)
     *                            1
     *             = -1 * ----------------- * x*x
     *                     (2*k+2)*(2*k+3)
    */
    term *= ( neg_x_x / (2*k+2) / (2*k+3) );
    k += 1;
  } while ( abs(term) >= eps*abs(series) );
  do {
    seriesR += term;
    term *= ( neg_x_x / (2*k+2) / (2*k+3) );
    k += 1;
  } while ( abs(term) >= epsR*abs(seriesR) );

  return series + seriesR;
}

/**
 * cosine function:
 *   use maclaurin_series
 * 
 * @param x 
 *   [-pi*2, pi*2], 
 *   inaccurate in (-inf, inf) because substraction in fmod(x, pi/2), 
 *   inaccurate at slightly great than pi/2 because ... in fmod(x, pi/2).
 * 
 * @return 
 *   [0, 1]
 * 
  * @derivative
 *   differentiate(cos, x) = -sin(x)
 * 
 * @integral
 *   integrate(cos(x), dx) = sin(x) + constant
 * 
 * @alternative
 *   cos(x) = cos(-x)             :even function
 * 
 *   cos(x) = cos(x + pi*2 * n)   :period function
 * 
 *   cos(2*x) = cos(x)*cos(x) - sin(x)*sin(x)   :double angle law
 * 
 *   cos(x/2) = sqrt((1 + cos(x))/2)            :half angle law
 * 
 *   cos(a+b) = cos(a)*cos(b) - sin(a)*sin(b)   :sum law
 * 
 *   cos(a-b) = cos(a)*cos(b) + sin(a)*sin(b)   :difference law
 * 
 *   cos(a) + cos(b) = 2 * cos((a+b)/2) * cos((a-b)/2)  :sum to product law
 * 
 *   cos(a) * cos(b) = 1/2 * ( cos(a-b) * cos(a+b) )    :product to sum law
 * 
 *   cos(x) = sum<k=0,inf>( pow(-1,k)/fact(2*k) * pow(x,2*k) )   :maclaurin_series, only even term, even is 2*k
*/
template<typename Number>
Number cos(const Number& x, const Number pi = static_cast<Number>(3.141592653589793)) {
  if ( abs(x) > pi/2 ) {
    // convergence optimize
    const Number pio2 = pi / 2;
#if defined __has_include && __has_include("rational.h")
    Number y = fmod_pi(x, rational64_t(1,2), pi);
#else 
    Number y = fmod(x, pio2);
#endif
    switch (static_cast<int>(trunc(x/pio2)) % 4) {
      case 0: return cos(y,pi);// 1->0
      case 1: return -sin(y,pi);// 0->-1
      case 2: return -cos(y,pi);// -1->0
      case 3: return  sin(y,pi);// 0->1
    }
  } else
  if ( x == 0 ) {
    // seriesR not convergence
    return static_cast<Number>(1);
  }

  assert( abs(x) <= pi/2 );
  // sum maclaurin_series
  const Number eps = std::numeric_limits<Number>::epsilon();
  Number series = 0;
  const Number epsR = static_cast<Number>(0.01);
  Number seriesR = 0;

  const Number neg_x_x = -(x*x);
  Number term = 1;
  Number k = 0;
  do {
    series += term;
    /**
     *  term[k+1]      -1^(k+1)      2*(k+1)     -1^k      2*k
     * ----------- = ------------ * x^       / -------- / x^
     *  term[k]       (2*(k+1))!                (2*k)!
     * 
     *                     (2*k)!       x^(2*k+2)
     *             = -1 * ---------- * ----------
     *                     (2*k+2)!     x^(2*k)
     * 
     *                            1
     *             = -1 * ----------------- * x*x
     *                     (2*k+1)*(2*k+2)
    */
    term *= ( neg_x_x / (2*k+1) / (2*k+2) ); 
    k += 1;
  } while ( abs(term) >= eps*abs(series) );
  do {
    seriesR += term;
    term *= ( neg_x_x / (2*k+1) / (2*k+2) ); 
    k += 1;
  } while ( abs(term) >= epsR*abs(seriesR) );

  return series + seriesR;
}

/**
 * tangent function
 *   ...
 * 
 * @param x 
 *   (-pi/2, pi/2)
 * 
 * @return
 *   (-inf, inf)
 * 
 * @derivative
 *   differentiate(tan,x) = sec(x)*sec(x)
 * 
 * @integral
 *   integrate(tan(x),dx) = -log(cos(x)) + constant
 * 
 * @alternative
 *   tan(x) = sin(x)/cos(x)
 * 
 *   tan(x) = sin(x*2)/(cos(x*2) + 1)
*/
template<typename Number> inline
Number tan(const Number& x, const Number pi = static_cast<Number>(3.141592653589793)) {
  return sin(x,pi) / cos(x,pi);
/**
 * @alternative of 'maclaurin_series'
 *   tan(x) = ....
 * 
 * @link
 *   "https://math.stackexchange.com/questions/2098941/bernoulli-numbers-taylor-series-expansion-of-tan-x"
 *   "https://math.stackexchange.com/questions/1546539/maclaurin-polynomial-of-tanx"
*/ 
}

/**
 * inverse sine function:
 *   use Maclaurin series
 * 
 * @param x 
 *   [-1, 1]
 * 
 * @param pi 
 *   constant
 * 
 * @return 
 *   [-pi/2, pi/2]
 * 
 * @series expansion
 *                                  1
 *   asin(x) = Integral[0,x]( ---------------, dx )                                                      :f(x) = integral( df(x), dx )
 *                             sqrt(1 - x*x)
 *
 *                                     -0.5
 *           = Integral[0,x]( (1 + -x*x)^  , dx )
 *
 *                                           -0.5 * -1.5 * -2.5 * ... * (-0.5 - k + 1)        k
 *           = Integral[0,x]( Sum[k=0,inf]( -------------------------------------------*(-x*x)^ ), dx )  :'binomial-series'
 *                                                            fact(k)
 *
 *                            -0.5 * -1.5 * -2.5 * ... * (-0.5 - k + 1)                         k         
 *           = Sum[k=0,inf]( ------------------------------------------- * Integral[0,x]( (-x*x)^, dx ) )
 *                                             fact(k)
 *
 *                            -0.5 * -1.5 * -2.5 * ... * (-0.5 - k + 1)        k                   2*k
 *           = Sum[k=0,inf]( ------------------------------------------- * (-1)^ * Integral[0,x]( x^  , dx ) )
 *                                             fact(k)
 *                        
 *                            -0.5 * -1.5 * -2.5 * ... * (-0.5 - k + 1)        k    x^(2*k+1)
 *           = Sum[k=0,inf]( ------------------------------------------- * (-1)^ * ----------- )         :integral constant C = 0
 *                                             fact(k)                                2*k+1
 *
 *                             0.5 * 1.5 * 2.5 * ... * -(-0.5 - k + 1)      x^(2*k+1)
 *           = Sum[k=0,inf]( ------------------------------------------- * ----------- )                 :(-1)^k * sign(numerator,k) == 1
 *                                             fact(k)                        2*k+1
 *
 *                            0.5 * 1.5 * 2.5 * ... * (k-1).5     x^(2*k+1)
 *           = Sum[k=0,inf]( --------------------------------- * ----------- )
 *                                       fact(k)                    2*k+1
 *
 *           = x + 0.5/3*x*x*x + 0.75/10*x*x*x*x*x + ...
*/
template<typename Number>
Number asin(const Number& x, const Number pi = static_cast<Number>(3.141592653589793)) {
  assert( x >= -1 && x <= 1 );
  if ( x > 0.707 ) {
    // convergence slow
    return pi/2 - asin(sqrt(1 - x*x));
  } else 
  if ( x < -0.707 ) {
    // convergence slow
    return asin(sqrt(1 - x*x)) - pi/2;
  } else 
  if ( x == 0 ) {
    // special case
    return static_cast<Number>(0);
  }

  // sum maclaurin_series
  const Number eps = static_cast<Number>(0.01);
  Number series = 0;
  const Number epsR = std::numeric_limits<Number>::epsilon();
  Number seriesR = 0;

  const Number half = static_cast<Number>(0.5);
  const Number x_x = x * x;
  Number term = x;
  Number n = 0;
  do {
    series += term;
    /**
     *  nth[n+1]     0.5 * 1.5 * 2.5 * ... * n.5     x^(2*(n+1)+1)     0.5 * 1.5 * 2.5 * ... * (n-1).5     x^(2*n+1)
     * ----------- = ----------------------------- * --------------- / --------------------------------- / -----------
     *  nth[n]                 fact(n+1)               2*(n+1)+1                   fact(n)                   2*n+1
     *           
     *                n.5     x^(2*n+3)     x^(2*n+1)
     *             = ----- * ----------- / -----------
     *                n+1       2*n+3         2*n+1
     *
     *                n.5     x*x*(2*n+1)
     *             = ----- * -------------
     *                n+1       2*n+3
    */
    term *= (n+half)/(n+1) * x_x*(2*n+1)/(2*n+3);
    n += 1;
  } while ( abs(term) >= eps*abs(series) );
  do {
    seriesR += term;
    term *= (n+half)/(n+1) * x_x*(2*n+1)/(2*n+3);
    n += 1;
  } while ( abs(term) >= epsR*abs(seriesR) );

  return series + seriesR;
}

/**
 * inverse cosine function:
 *   use Maclaurin series

 * @param x 
 *   [-1, 1]

 * @param pi 
 *   constant
 * 
 * @return 
 *   [0, pi]
 * 
 * @formula
 *   acos(x) = pi/2 - asin(x)
*/
template<typename Number>
Number acos(const Number& x, const Number pi = static_cast<Number>(3.141592653589793)) {
  return pi/2 - asin(x);
}

/**
 * inverse tangent function
 * 
 * @param x 
 *   (-inf, inf)
 * 
 * @return
 *   [-pi/2, pi/2]
 * 
 * @Euler's XXXX-series
 *                            2^(2*k) * k! * k!       x^(2*k + 1)
 *   atan(x) = sum<k=0,inf>( ------------------- * ------------------- )
 *                                (2*k + 1)!        (1 + x*x)^(k + 1)
 * 
 * @line 
 *   "https://en.wikipedia.org/wiki/Inverse_trigonometric_functions"
*/
template<typename Number>
Number atan(const Number& x) {
  if ( x < 0 ) {
    // optimize convergence, remove abs(...)
    return -atan(abs(x));
  } else if ( x > 0.35 ) {
    // convergence slow
    return atan(x / (1 + sqrt(1 + x*x))) * 2;
  } else if ( x == 0 ) {
    // not convergence
    return static_cast<Number>(0);
  }

  assert( 0 < x && x <= 0.35 );
  const Number eps = std::numeric_limits<Number>::epsilon();
  const Number xx_div_1pxx = x*x/(1+x*x);
  Number series = 0;
  Number term = x/(1+x*x);
  Number k = 0;
	do {
		series += term;
    /**
     *	nth[k+1]     2^(2*(k+1)) * (k+1)! * (k+1)!       x^(2*(k+1) + 1)       2^(2*k) * k! * k!       x^(2*k + 1)
     * ----------- = ------------------------------- * --------------------- / ------------------- / -------------------
     *	nth[k]              (2*(k+1) + 1)!             (1 + x*x)^(k+1 + 1)         (2*k + 1)!        (1 + x*x)^(k + 1)
     *                                                                             
     *                2^(2*k+2) * (k+1)! * (k+1)!     (2*k + 1)!     x^(2*k + 3)     (1+x*x)^(k + 1)
     *             = ----------------------------- * ------------ * ------------- * -----------------
     *                2^(2*k) * k! * k!               (2*k + 3)!     x^(2*k + 1)     (1+x*x)^(k + 2)
     * 
     *                                            1                     1
     *             = 2^2 * (k+1) * (k+1) * ----------------- * x^2 * -------
     *                                      (2*k+2)*(2*k+3)           1+x*x
     * 
     *                 4*(k+1)*(k+1)       x*x
     *             = ----------------- * -------
     *                (2*k+2)*(2*k+3)     1+x*x
    */
		term *= ( 4*(k+1)*(k+1) / (2*k+2)/(2*k+3) * xx_div_1pxx );
		k += 1;
	} while ( term >= eps * series );
		
	return series;
}

/**
 * inverse tangent function 
 *   for two arguments
 * @return 
 *   (-pi, pi]
*/
template<typename Number> inline
Number atan2(const Number& y, const Number& x, const Number pi = static_cast<Number>(3.141592653589793)) {
  if (x > 0) {
    return atan(y / x);
  } else if (x < 0 && y >= 0) {
    return atan(y / x) + pi;
  } else if (x < 0 && y < 0) {
    return atan(y / x) - pi;
  } else if (x == 0 && y > 0) {
    return pi / 2;
  } else if (x == 0 && y < 0) {
    return -pi / 2;
  } else  /* x == 0 && y == 0 */ {
    return std::numeric_limits<Number>::signaling_NaN();
  }

  /*if (x > 0) {
    return atan(y / (sqrt(x*x + y*y) + x)) * 2;
  } else if (x <= 0 && y != 0) {
    return atan((sqrt(x*x + y*Y) - x) / y) * 2;
  } else if (x < 0 && y == 0) {
    return pi;
  } else {
    return std::numeric_limits<Number>::signaling_NaN();
  }*/
}


using _CSTD pow;
inline float32_t pow(float32_t x, float32_t power) { return _CSTD powf(x, power); }
inline float64_t pow(float64_t x, float64_t power) { return _CSTD pow(x, power); }
using _CSTD sqrt;// literal
inline float32_t sqrt(float32_t x) { return _CSTD sqrtf(x); }
inline float64_t sqrt(float64_t x) { return _CSTD sqrt(x); }
using _CSTD cbrt;// literal
inline float32_t cbrt(float32_t x) { return _CSTD cbrtf(x); }
inline float64_t cbrt(float64_t x) { return _CSTD cbrt(x); }
using _CSTD exp;
inline float32_t exp(float32_t x) { return _CSTD expf(x); }
inline float64_t exp(float64_t x) { return _CSTD exp(x); }
using _CSTD log;
inline float32_t log(float32_t x) { return _CSTD logf(x); }
inline float64_t log(float64_t x) { return _CSTD log(x); }

using _CSTD sin;
inline float32_t sin(float32_t x) { return _CSTD sinf(x); }
inline float64_t sin(float64_t x) { return _CSTD sin(x); }
using _CSTD cos;
inline float32_t cos(float32_t x) { return _CSTD cosf(x); }
inline float64_t cos(float64_t x) { return _CSTD cos(x); }
using _CSTD tan;
inline float32_t tan(float32_t x) { return _CSTD tanf(x); }
inline float64_t tan(float64_t x) { return _CSTD tan(x); }
using _CSTD asin;// literal
inline float32_t asin(float32_t x) { return _CSTD asinf(x); }
inline float64_t asin(float64_t x) { return _CSTD asin(x); }
using _CSTD acos;// literal
inline float32_t acos(float32_t x) { return _CSTD acosf(x); }
inline float64_t acos(float64_t x) { return _CSTD acos(x); }
using _CSTD atan;// literal
inline float32_t atan(float32_t x) { return _CSTD atanf(x); }
inline float64_t atan(float64_t x) { return _CSTD atan(x); }


/* wrapping a lattice-function
* latticef(domain:[any]) -> range:[any]
* normalized_latticef(domain:[0,1]x[0,1]) -> range:[any]
*/
template<typename Lattice> requires requires(Lattice __f) { __f(size_t(), size_t()); }
class NormalizedSampler2D {
public:    
  using lattice_type = Lattice;
  using result_type  = decltype(Lattice()(size_t(),size_t()));
	
  const lattice_type* plattice;
  size_t lattice_col_backindex;
  size_t lattice_row_backindex;

  template<typename Ty>
  result_type operator()(Ty u, Ty v) const {
    assert( 0 <= u && u <= 1 );
    assert( 0 <= v && v <= 1 );
    return bilersmp(*plattice, u*lattice_col_backindex, v*lattice_row_backindex);
  }

public:
  NormalizedSampler2D() = default;
  NormalizedSampler2D(const lattice_type& _lattice, size_t _rows, size_t _cols)
    : plattice(&_lattice), lattice_col_backindex(_cols-1), lattice_row_backindex(_rows-1) {}
  size_t rows() const { return lattice_row_backindex + 1; }
  size_t cols() const { return lattice_col_backindex + 1; }
};


/* wrapping a lattice-function
*/
template<typename Lattice> requires requires(Lattice __f) { __f(size_t(), size_t(), size_t()); }
class NormalizedSampler3D {
public:
  using lattice_type = Lattice;
  using result_type  = decltype(Lattice()(size_t(),size_t(),size_t()));
		
  const lattice_type* plattice;
  size_t lattice_col_backindex;
  size_t lattice_row_backindex;
  size_t lattice_slice_backindex;
    
  template<typename Ty>
  result_type operator()(Ty u, Ty v, Ty w) const {
    assert( 0 <= u && u <= 1 );
    assert( 0 <= v && v <= 1 );
    assert( 0 <= w && w <= 1 );
    return trilersmp(*plattice, u*lattice_col_backindex, v*lattice_row_backindex, w*lattice_slice_backindex);
  }

public:
  NormalizedSampler3D() = default;
  NormalizedSampler3D(const lattice_type& _lattice, size_t _rows, size_t _cols, size_t _slices)
    : plattice(&_lattice), lattice_col_backindex(_cols-1), lattice_row_backindex(_rows-1), lattice_slice_backindex(_slices-1) {}
  size_t rows() const { return lattice_row_backindex + 1; }
  size_t cols() const { return lattice_col_backindex + 1; }
  size_t slices() const { return lattice_slice_backindex + 1; }
};


/* wrapping a lattice-function
* latticef(domain:[any]) -> range:[any]
* heightmap(domain:[horizontal_lowest,horizontal_max]) -> range:[vertical_lowest,vertical_max]
*/
template<typename Lattice, typename Length = double> requires requires(Lattice __f) { __f(size_t(), size_t()); }
struct HeightmapSampler {
  using lattice_type = Lattice;
  using value_type  = decltype(Lattice()(size_t(),size_t()));
  using length_type = Length;

  const lattice_type& lattice;
  size_t lattice_row_backindex;
  size_t lattice_col_backindex;
  value_type lattice_value_lowest;
  value_type lattice_value_max;

  length_type horizontal_lowest[2];
  length_type horizontal_max[2];
  length_type vertical_lowest;
  length_type vertical_max;

  template<typename Ty>
  length_type operator()(const Ty& x, const Ty& z) const {
    assert( horizontal_lowest[0] <= x && x <= horizontal_max[0] );
    assert( horizontal_lowest[1] <= z && z <= horizontal_max[1] );

    Ty u = remap(x, horizontal_lowest[0],horizontal_max[0], size_t(0),lattice_row_backindex);

    Ty v = remap(z, horizontal_lowest[1],horizontal_max[1], size_t(0),lattice_col_backindex);

    return remap(bilersmp(lattice, u, v), lattice_value_lowest,lattice_value_max, vertical_lowest,vertical_max);
  }

public:
  HeightmapSampler(const lattice_type& _lattice, size_t _rows, size_t _cols, value_type _lowest, value_type _max,
    length_type arg0_lowest, length_type arg0_max,
    length_type arg1_lowest, length_type arg1_max,
    length_type result_lowest, length_type result_max) : lattice(_lattice) {
    lattice_row_backindex = _rows - 1;
    lattice_col_backindex = _cols - 1;
    lattice_value_lowest = _lowest;
    lattice_value_max = _max;
    horizontal_lowest[0] = arg0_lowest;
    horizontal_max[0] = arg0_max;
    horizontal_lowest[1] = arg1_lowest;
    horizontal_max[1] = arg1_max;
    vertical_lowest = result_lowest;
    vertical_max = result_max;
  }

  HeightmapSampler(const lattice_type& _lattice, size_t _rows, size_t _cols,
    length_type arg0_lowest, length_type arg0_max,
    length_type arg1_lowest, length_type arg1_max,
    length_type result_lowest, length_type result_max)
    : HeightmapSampler(_lattice, _rows, _cols, 0, 1, arg0_lowest, arg0_max, arg1_lowest, arg1_max, result_lowest, result_max) {}
		
  size_t rows() const { 
    return lattice_row_backindex + 1;
  }
		
  size_t cols() const { 
    return lattice_col_backindex + 1;
  }
};


// undeterminant

// hypergeometric series( Gauss )
template<typename Number>
Number hypergeometric_series(Number a, Number b, Number c, Number z, size_t n = 1024){
  /* hypergeometric series( Gauss )
                  a*b     a*(a+1)*b*(b+1)       a*(a+1)*(a+2)*b*(b+1)*(b+2)               fact(i,a)*fact(i,b)
  * series = 1 + ---*z + ---------------*z*z + ---------------------------*z*z*z + ... + -------------------*pow(z,i)
                  1*c       1*2*c*(c+1)             1*2*3*c*(c+1)*(c+2)                    fact(i)*fact(i,c)
    abs(z) < 1, absolutely converges
    abs(z) == 1, requires real(c-a-b) > 0

  * application:
                    dd'y'                 d'y'
    solve [ z*(1-z)*----- + (c-(a+b+1)*z)*---- - a*b*y = 0 ], result'y' is hypergeometric_series(a,b,c,z)
                      dzz                   dz
    asin(z) = z * hypergeometric_series(0.5, 0.5, 1.5, z*z)
    atan(z) = z * hypergeometric_series(0.5, 1.0, 1.5, -z*z)
  */
  
  assert( abs(z) < 1 || (abs(z) == 1 && (c-a-b) > 0) );

  const Number eps = std::numeric_limits<Number>::epsilon();
  Number result = 0;
  Number term = 1;
  size_t i = 0;
	do {
    result += term;
    term = term * (a+i)*(b+i)/(1+i)/(c+i)*z;
	} while (++i != n && abs(term) >= eps*max(abs(result),eps));

	return result;
}

// gamma function( Euler )
template<typename Number>
Number gamma(Number z, Number r, size_t n, Number pi, Number e) {
  /* gamma function approximation( Lanczos )
                                 z   -t
  gamma(z+1) = integral[0,inf]( t^ * e^, dt )


  Substitution 't' = u*a, a != 0
                          z   -u*a
  = integral[0,inf]( (u*a)^ * e^  , dt )              :t = u*a
                              z   -u*a
  = integral[0/a,inf/a]( (u*a)^ * e^ * a, a*du )      :dt = a*du, u = t/a
                      z    z   -u*a               z+1
  = integral[0,inf]( u^ * a^ * e^   * a, du ) * a^    :simplify
                      z   -u*a           z+1
  = integral[0,inf]( u^ * e^  , du ) * a^             :simplify


  Replacing 'a' by z+r+1, since 'z' is a constant with process of integration
                      z   -u*(z+r+1)                z+1
  = integral[0,inf]( u^ * e^        , du ) * (z+r+1)^


  Substituation 'u' = 1 - log(v)
                               z    -(1-log(v))*(z+r+1)                z+1
  = integral[0,inf]( (1-log(v))^ * e^                  , du ) * (z+r+1)^           :u = 1 - log(v)

                               z    -(z+r+1)    log(v)*(z+r+1)                z+1
  = integral[0,inf]( (1-log(v))^ * e^        * e^             , du ) * (z+r+1)^    :simplify exponential distribution

                               z    log(v)*(z+r+1)                z+1   -(z+r+1)
  = integral[0,inf]( (1-log(v))^ * e^             , du ) * (z+r+1)^   * e^         :simplify move out integration

                               z    z    r                    z+1   -(z+r+1)
  = integral[0,inf]( (1-log(v))^ * v^ * v^ * v, du ) * (z+r+1)^   * e^             :simplify

                                             z    z    r                         z+1   -(z+r+1)
  = integral[exp(1-0),exp(1-inf)]( (1-log(v))^ * v^ * v^ * v, -1/v*dv ) * (z+r+1)^   * e^         :du = -1/v * dv, v = exp(1 - u)

                             z    z    r                           z+1   -(z+r+1)
  = integral[e,0]( (1-log(v))^ * v^ * v^ * v * -1/v, dv ) * (z+r+1)^   * e^          :simplify

                                 z    r                           z+1   -(z+r+1)
  = integral[e,0]( ((1-log(v))*v)^ * v^ * v * -1/v, dv ) * (z+r+1)^   * e^         :simplify

                                 z    r                     z+1   -(z+r+1)
  = integral[e,0]( ((1-log(v))*v)^ * v^ * -1, dv ) * (z+r+1)^   * e^         :simplify

                                 z    r                z+1   -(z+r+1)
  = integral[0,e]( ((1-log(v))*v)^ * v^, dv ) * (z+r+1)^   * e^         :simplify, note: r > 0 may be 1

  */

  z = z - 1;
    
    /*
          k                                                                             k*pi*x                                  
alpha = -1^ * (2/pi) * Integral[-1,1]( ( alpha(0,r)*0.5 + sum[k=1,inf]( alpha(k,r)*cos(--------) ) )
                                                                                         pi/2
                                                              2*j
                                       * sum[j=0,k]( C2j2k * x^   )
                                                1
                                       * ---------------
                                          sqrt(1 - x*x)
                                      ,dx )

		  k                                                                             k*pi*x
alpha = -1^ * (2/pi) * Integral[-1,1]( ( alpha(0,r)*0.5 + sum[k=1,inf]( alpha(k,r)*cos(--------) ) )
																						 pi/2
										   k                             2 j
									   * -1^ * sum[j=0,k]( C2j2k * (1 - x^)^ )
												1
									   * ---------------
										  sqrt(1 - x*x)
									  ,dx )

....
               k                 r                     j       fact(k+j-1)            e     j+0.5
alpha(k,r) = -1^ * sqrt(2/pi) * e^ * k * Sum[j=0,k]( -1^ * ------------------- * (---------)^     )
                                                            fact(k-j)*fact(j)      j+r+0.5
*/
  auto alpha = [pi,e](size_t k, Number r) {
    if (k == 0) {
      return sqrt(2*e / (pi*(r+0.5))) * exp(r);
    }
    return pow(-1,k) * sqrt(2/pi) * exp(r) * k
      * sum(size_t(0),k,[k,r,e](size_t j){ return fact(k+j-1)/fact(k-j)/fact(j) * pow(e/(j+r+0.5),j+0.5) * pow(-1,j); });
  };

  Number Fr = alpha(0,r) / 2;
  Number c = z/(z+1);
  for (size_t k = 1; k != n; ++k) {
    Fr += alpha(k,r) * c;
    c = c * (z-k) / (z+k+1);
  }

  return pow(z+r+0.5,z+0.5) * exp(-(z+r+0.5)) * sqrt(pi*2) * Fr;
}
	
}// namespace calculation