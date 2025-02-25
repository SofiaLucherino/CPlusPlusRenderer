#if !defined (GRAPHICS_MATH_H)

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>


global f32 Pi32 = 3.14159265359f;

union f32_x4 
{
	struct 
	{
		__m128 Vals;
	};

	f32 e[4];
};

union i32_x4
{
	struct 
	{
		__m128i Vals;
	};
	
	i32 e[4];
};


union v2 
{
	struct 
	{
		f32 x, y;
	};

	f32 e[2];
};

union v2_x4
{
	struct
	{
		f32_x4 x, y;
	};

	f32_x4 e[2];
};

union v2i
{
	struct
	{
		i32 x, y;
	};

	i32 e[2];
};

union v2i_x4
{
	struct
	{
		i32_x4 x, y;
	};

	i32_x4 e[2];
};

union v3
{
	struct
	{
		f32 x, y, z;
	};

	struct
	{
		f32 r, g, b;
	};

	struct
	{
		v2 xy;
		f32 Ignored0;
	};

	struct
	{
		f32 Ignored1;
		v2 yz;
	};

	f32 e[3];
};

union v3_x4
{
	struct
	{
		f32_x4 x, y, z;
	};

	struct
	{
		f32_x4 r, g, b;
	};

	struct
	{
		v2_x4 xy;
		f32_x4 Ignored0;
	};

	struct
	{
		f32_x4 Ignored1;
		v2_x4 yz;
	};

	f32_x4 e[3];
};

union v4 
{
	struct 
	{
		f32 x, y, z, w;
	};

	struct
	{
		f32 r, g, b, a;
	};

	struct 
	{
		v3 xyz;
		f32 Ignored0;
	};

	struct
	{
		v2 xy;
		v2 Ignored1;
	};

	f32 e[4];
};

union m4 
{
	v4 v[4];
	f32 e[16];
};

#define WIN32_GRAPHICSPRACTISE_H
#endif