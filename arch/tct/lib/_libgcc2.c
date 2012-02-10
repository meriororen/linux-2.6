/* Arithmetic functions taken from libgcc2 for tct */

typedef		 int QItype	__attribute__ ((mode (QI)));
typedef unsigned int UQItype	__attribute__ ((mode (QI)));
typedef		 int HItype	__attribute__ ((mode (HI)));
typedef unsigned int UHItype	__attribute__ ((mode (HI)));
typedef 	 int SItype	__attribute__ ((mode (SI)));
typedef unsigned int USItype	__attribute__ ((mode (SI)));
typedef		 int DItype	__attribute__ ((mode (DI)));
typedef unsigned int UDItype	__attribute__ ((mode (DI)));

typedef int cmp_return_type __attribute__((mode (__libgcc_cmp_return__)));
typedef int shift_count_type __attribute__((mode (__libgcc_shift_count__)));


#define BITS_PER_UNIT 8

#define W_TYPE_SIZE (4 * BITS_PER_UNIT)
#define Wtype	SItype
#define UWtype	USItype
#define HWtype	SItype
#define UHWtype	USItype
#define DWtype	DItype
#define UDWtype	UDItype
#define __NW(a,b)	__ ## a ## si ## b
#define __NDW(a,b)	__ ## a ## di ## b

extern UDWtype __umulsidi3 (UWtype, UWtype);

#define Wtype_MAX ((Wtype)(((UWtype)1 << (W_TYPE_SIZE - 1)) - 1))
#define Wtype_MIN (- Wtype_MAX - 1)

#define Wtype_MAXp1_F	0x1p32f

extern DWtype __muldi3 (DWtype, DWtype);
extern DWtype __divdi3 (DWtype, DWtype);
extern UDWtype __udivdi3 (UDWtype, UDWtype);
extern UDWtype __umoddi3 (UDWtype, UDWtype);
extern DWtype __moddi3 (DWtype, DWtype);
extern DWtype __negdi2 (DWtype);
extern DWtype __lshrdi3 (DWtype, shift_count_type);
extern DWtype __ashldi3 (DWtype, shift_count_type);
extern DWtype __ashrdi3 (DWtype, shift_count_type);

// little endian
struct DWstruct {
	Wtype low, high;
};

typedef union
{
  struct DWstruct s;
  DWtype ll;
} DWunion;

DWtype
__negdi2 (DWtype u)
{
  const DWunion uu = {.ll = u};
  const DWunion w = { {.low = -uu.s.low,
		       .high = -uu.s.high - ((UWtype) -uu.s.low > 0) } };

  return w.ll;
}

#define __ll_B ((UWtype) 1 << (W_TYPE_SIZE / 2))
#define __ll_lowpart(t) ((UWtype) (t) & (__ll_B - 1))
#define __ll_highpart(t) ((UWtype) (t) >> (W_TYPE_SIZE / 2))
#define umul_ppmm(w1, w0, u, v)						\
  do {									\
    UWtype __x0, __x1, __x2, __x3;					\
    UHWtype __ul, __vl, __uh, __vh;					\
									\
    __ul = __ll_lowpart (u);						\
    __uh = __ll_highpart (u);						\
    __vl = __ll_lowpart (v);						\
    __vh = __ll_highpart (v);						\
									\
    __x0 = (UWtype) __ul * __vl;					\
    __x1 = (UWtype) __ul * __vh;					\
    __x2 = (UWtype) __uh * __vl;					\
    __x3 = (UWtype) __uh * __vh;					\
									\
    __x1 += __ll_highpart (__x0);/* this can't give carry */		\
    __x1 += __x2;		/* but this indeed can */		\
    if (__x1 < __x2)		/* did we get it? */			\
      __x3 += __ll_B;		/* yes, add it in the proper pos.  */	\
									\
    (w1) = __x3 + __ll_highpart (__x1);					\
    (w0) = __ll_lowpart (__x1) * __ll_B + __ll_lowpart (__x0);		\
  } while (0)

#define __umulsidi3(u, v) \
  ({DWunion __w;							\
    umul_ppmm (__w.s.high, __w.s.low, u, v);				\
    __w.ll; })

DWtype
__muldi3 (DWtype u, DWtype v)
{
  const DWunion uu = {.ll = u};
  const DWunion vv = {.ll = v};
  DWunion w = {.ll = __umulsidi3 (uu.s.low, vv.s.low)};

  w.s.high += ((UWtype) uu.s.low * (UWtype) vv.s.high
	       + (UWtype) uu.s.high * (UWtype) vv.s.low);

  return w.ll;
}

DWtype
__lshrdi3 (DWtype u, shift_count_type b)
{
  if (b == 0)
    return u;

  const DWunion uu = {.ll = u};
  const shift_count_type bm = (sizeof (Wtype) * BITS_PER_UNIT) - b;
  DWunion w;

  if (bm <= 0)
    {
      w.s.high = 0;
      w.s.low = (UWtype) uu.s.high >> -bm;
    }
  else
    {
      const UWtype carries = (UWtype) uu.s.high << bm;

      w.s.high = (UWtype) uu.s.high >> b;
      w.s.low = ((UWtype) uu.s.low >> b) | carries;
    }

  return w.ll;
}

DWtype
__ashldi3 (DWtype u, shift_count_type b)
{
  if (b == 0)
    return u;

  const DWunion uu = {.ll = u};
  const shift_count_type bm = (sizeof (Wtype) * BITS_PER_UNIT) - b;
  DWunion w;

  if (bm <= 0)
    {
      w.s.low = 0;
      w.s.high = (UWtype) uu.s.low << -bm;
    }
  else
    {
      const UWtype carries = (UWtype) uu.s.low >> bm;

      w.s.low = (UWtype) uu.s.low << b;
      w.s.high = ((UWtype) uu.s.high << b) | carries;
    }

  return w.ll;
}

DWtype
__ashrdi3 (DWtype u, shift_count_type b)
{
  if (b == 0)
    return u;

  const DWunion uu = {.ll = u};
  const shift_count_type bm = (sizeof (Wtype) * BITS_PER_UNIT) - b;
  DWunion w;

  if (bm <= 0)
    {
      /* w.s.high = 1..1 or 0..0 */
      w.s.high = uu.s.high >> (sizeof (Wtype) * BITS_PER_UNIT - 1);
      w.s.low = uu.s.high >> -bm;
    }
  else
    {
      const UWtype carries = (UWtype) uu.s.high << bm;

      w.s.high = uu.s.high >> b;
      w.s.low = ((UWtype) uu.s.low >> b) | carries;
    }

  return w.ll;
}
