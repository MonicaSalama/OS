#ifndef THREADS_PINTOS_MATH_H
#define THREADS_PINTOS_MATH_H

int to_fixed_point(int n);
int to_int_rounded_towards_zero(int x);
int to_int_rounded_to_nearest(int x);
int add(int x, int y);
int add_fixed_point_to_int(int x, int n);
int sub(int x, int y);
int sub_int_from_fixed_point(int x, int n);
int mult_fixed_points(int x, int y);
int mult_fixed_point_and_int(int x, int y);
int div_fixed_points(int x, int y);
int div_fixed_points_by_int(int x, int n);

#endif
