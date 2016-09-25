#include <stdint.h>
#include <stddef.h>
#include <debug.h>
#include "threads/pintos_math.h"

#define factor 16384

int
to_fixed_point(int n)
{
  return n * factor;
}

int
to_int_rounded_towards_zero(int x)
{
  return x / factor;
}

int
to_int_rounded_to_nearest(int x)
{
  if (x >= 0)
      return (x + factor / 2) / factor;
  else
      return (x - factor / 2) / factor;
}

int
add(int x, int y)
{
  return x + y;
}

int
add_fixed_point_to_int(int x, int n)
{
  return x + to_fixed_point(n);
}

int
sub(int x, int y)
{
  return x - y;
}

int
sub_int_from_fixed_point(int x, int n)
{
  return x - to_fixed_point(n);
}

int
mult_fixed_points(int x, int y)
{
  return (((int64_t) x) * y) / factor;
}

int
mult_fixed_point_and_int(int x, int n)
{
  return x * n;
}

int
div_fixed_points(int x, int y)
{
  return ((int64_t) x) * factor / y;
}

int
div_fixed_points_by_int(int x, int n)
{
  return x / n;
}
