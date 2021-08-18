#include <math.h>

struct vec4
{
    float x, y, z;
    float w = 1.0f;
};

struct tri
{
    vec4 v[3];
    float brightness;
};

struct mat4
{
    float m[4][4];
    // | a1 a2 a3 a4 | ->[[a1, b1, c1, d1],
    // | b1 b2 b3 b4 | -> [a2, b2, c2, d2],
    // | c1 c2 c3 c4 | -> [a3, b3, c3, d3],
    // | d1 d2 d3 d4 | -> [a4, b4, c4, d4]]
};

// 2.2x faster than normal
void add(vec4 *out, vec4 *a, vec4 *b)
{
    __asm__ volatile
    (
    "movaps (%0), %%xmm0\n\t"  // xmm0:   a.x  |  a.y  |  a.z  |  a.w
    "addps (%1), %%xmm0\n\t"   // xmm0: a.x+b.x|a.y+b.y|a.z+b.z|a.w+b.w
    "movaps %%xmm0, (%2)"      // out <-- xmm0
    :
    : "r" (a), "r" (b), "r" (out)
    : "%xmm0");
}
// 2.2x faster than normal
void sub(vec4 *out, vec4 *a, vec4 *b)
{
    __asm__ volatile
    (
    "movaps (%0), %%xmm0\n\t"  // xmm0:   a.x  |  a.y  |  a.z  |  a.w
    "subps (%1), %%xmm0\n\t"   // xmm0: a.x-b.x|a.y-b.y|a.z-b.z|a.w-b.w
    "movaps %%xmm0, (%2)"      // out <-- xmm0
    :
    : "r" (a), "r" (b), "r" (out)
    : "%xmm0");
}
// 1.85x faster than normal
void mul(vec4 *out, vec4 *a, float b)
{
    __asm__ volatile
    (
    "movd %1, %%xmm1\n\t"            // xmm1: b|0|0|0
    "shufps $0, %%xmm1, %%xmm1\n\t"  // xmm1: b|b|b|b
    "movaps (%0), %%xmm0\n\t"        // xmm0:  a.x | a.y | a.z | a.w
    "mulps %%xmm1, %%xmm0\n\t"       // xmm0: a.x*b|a.y*b|a.z*b|a.w*b
    "movaps %%xmm0, (%2)"            // out <-- xmm0
    :
    : "r" (a), "r" (b), "r" (out)
    : "%xmm0", "%xmm1");
}
// 4.1x faster than normal
void div(vec4 *out, vec4 *a, float b)
{
    __asm__ volatile
    (
    "movd %1, %%xmm1\n\t"            // xmm1: b|0|0|0
    "shufps $0, %%xmm1, %%xmm1\n\t"  // xmm1: b|b|b|b
    "movaps (%0), %%xmm0\n\t"        // xmm0:  a.x | a.y | a.z | a.w
    "divps %%xmm1, %%xmm0\n\t"       // xmm0: a.x/b|a.y/b|a.z/b|a.w/b
    "movaps %%xmm0, (%2)"            // out <-- xmm0
    :
    : "r" (a), "r" (b), "r" (out)
    : "%xmm0", "%xmm1");
}
// 2.5x faster than normal
void normalize(vec4 *out, vec4 *a)
{
    __asm__ volatile
    (
    "movaps (%0), %%xmm0\n\t"     // xmm0: a.x|a.y|a.z|a.w
    "movaps %%xmm0, %%xmm1\n\t"   // xmm1 <-- xmm0
    "mulps %%xmm1, %%xmm1\n\t"    // xmm1:     a.x*a.x    |    a.y*a.y    |    a.z*a.z    |    a.w*a.w
    "haddps %%xmm1, %%xmm1\n\t"   // xmm1: a.x*a.x+a.y*a.y|a.z*a.z+a.w*a.w|a.x*a.x+a.y*a.y|a.z*a.z+a.w*a.w
    "haddps %%xmm1, %%xmm1\n\t"   // xmm1: (a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w) repeated 4 times
    "rsqrtps %%xmm1, %%xmm1\n\t"  // xmm1: 1 / sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w) repeated 4 times
    "mulps %%xmm1, %%xmm0\n\t"    // xmm0 <-- xmm0 / sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w)
    "movaps %%xmm0, (%1)"         // out <-- xmm0
    :
    : "r" (a), "r" (out)
    : "%xmm0", "%xmm1");
}
// marginally faster than normal
float dot(vec4 *a, vec4 *b)
{
    float out;
    __asm__ volatile
    (
    "movaps (%0), %%xmm0\n\t"     // xmm0: a.x|a.y|a.z|a.w
    "mulps (%1), %%xmm0\n\t"      // xmm0: a.x*b.x|a.y*b.y|a.z*b.z|a.w*b.w
    "haddps %%xmm0, %%xmm0\n\t"   // xmm0: a.x*b.x+a.y*b.y|a.z*b.z+a.w*b.w|a.x*b.x+a.y*b.y|a.z*b.z+a.w*b.w
    "haddps %%xmm0, %%xmm0\n\t"   // xmm0: dot(a, b) repeated 4 times
    "movss %%xmm0, (%2)"
    :
    : "r" (a), "r" (b), "r" (&out)
    : "%xmm0");
    return out;
}
// cant do a fast assembly implementation
void cross(vec4 *out, vec4 *a, vec4 *b)
{
    out->x = a->y*b->z-a->z*b->y;
    out->y = a->z*b->x-a->x*b->z;
    out->z = a->x*b->y-a->y*b->x;
}

void set_mat(mat4 *out,
float a1, float a2, float a3, float a4,
float b1, float b2, float b3, float b4,
float c1, float c2, float c3, float c4,
float d1, float d2, float d3, float d4)
{
	out->m[0][0] = a1; out->m[0][1] = b1; out->m[0][2] = c1; out->m[0][3] = d1;
	out->m[1][0] = a2; out->m[1][1] = b2; out->m[1][2] = c2; out->m[1][3] = d2;
	out->m[2][0] = a3; out->m[2][1] = b3; out->m[2][2] = c3; out->m[2][3] = d3;
	out->m[3][0] = a4; out->m[3][1] = b4; out->m[3][2] = c4; out->m[3][3] = d4;
}

// can't tell if it's faster than normal, but it takes up less space
// normal: 215 bytes -Ofast -msse3 (doesnt include ret)
// this: 81 bytes -Ofast -msse3 (doesnt include ret)
// normal: 442 bytes no flags (doesnt include stack setup)
// this: 81 bytes no flags (doesnt include stack setup)
// also the first function that require unrolling loops and removing dependency chains
// to catch up to speed with the normal version
void mul_mat_vec(vec4 *out, mat4 *mat, vec4 *vec)
{
    __asm__ volatile
    (
    "movaps (%1), %%xmm0\n\t"    // xmm0: vec.x|vec.y|vec.z|vec.w
    "movaps (%0), %%xmm1\n\t"    // xmm1: a1|b1|c1|d1
    "movaps 16(%0), %%xmm2\n\t"  // xmm2: a2|b2|c2|d2
    "movaps 32(%0), %%xmm3\n\t"  // xmm3: a3|b3|c3|d3
    "movaps 48(%0), %%xmm4\n\t"  // xmm4: a4|b4|c4|d4
    "mulps %%xmm0, %%xmm1\n\t"   // xmm1: a1 * vec.x|b1 * vec.y|c1 * vec.z|d1 * vec.w
    "mulps %%xmm0, %%xmm2\n\t"   // xmm2: a2 * vec.x|b2 * vec.y|c2 * vec.z|d2 * vec.w
    "mulps %%xmm0, %%xmm3\n\t"   // xmm3: a3 * vec.x|b3 * vec.y|c3 * vec.z|d3 * vec.w
    "mulps %%xmm0, %%xmm4\n\t"   // xmm4: a4 * vec.x|b4 * vec.y|c4 * vec.z|d4 * vec.w
    "haddps %%xmm1, %%xmm1\n\t"
    "haddps %%xmm2, %%xmm2\n\t"
    "haddps %%xmm3, %%xmm3\n\t"
    "haddps %%xmm4, %%xmm4\n\t"
    "haddps %%xmm1, %%xmm1\n\t"  // xmm1: dot(m1, vec) repeated 4 times
    "haddps %%xmm2, %%xmm2\n\t"  // xmm2: dot(m2, vec) repeated 4 times
    "haddps %%xmm3, %%xmm3\n\t"  // xmm3: dot(m3, vec) repeated 4 times
    "haddps %%xmm4, %%xmm4\n\t"  // xmm4: dot(m4, vec) repeated 4 times
    "movss %%xmm1, (%2)\n\t"
    "movss %%xmm2, 4(%2)\n\t"
    "movss %%xmm3, 8(%2)\n\t"
    "movss %%xmm4, 12(%2)\n\t"
    :
    : "r" (mat), "r" (vec), "r" (out)
    : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4");
}

void mul_mat_mat(mat4 *out, mat4 *a, mat4 *b)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            out->m[i][j] = a->m[0][j]*b->m[i][0]
                          +a->m[1][j]*b->m[i][1]
                          +a->m[2][j]*b->m[i][2]
                          +a->m[3][j]*b->m[i][3];
}

void rotx(mat4 *out, float angle)
{
    float c = cosf(angle), s = sinf(angle);
    set_mat(out,
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f,  c,   -s,   0.0f,
    0.0f,  s,    c,   0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
}

void roty(mat4 *out, float angle)
{
    float c = cosf(angle), s = sinf(angle);
    set_mat(out,
      c,  0.0f,  s,   0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
     -s,  0.0f,  c,   0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
}

void rotz(mat4 *out, float angle)
{
    float c = cosf(angle), s = sinf(angle);
    set_mat(out,
     c,   -s,   0.0f, 0.0f,
     s,    c,   0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);
}

void translation(mat4 *out, float x, float y, float z)
{
    set_mat(out,
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    x,y,z,1);
}

void point_at(mat4 *out, vec4 *p, vec4 *t, vec4 *up)
{
    vec4 f, r, u, a;
    sub(&f, t, p);
    normalize(&f, &f);
    mul(&a, &f, dot(up, &f));
    sub(&u, up, &a);
    normalize(&u, &u);
    cross(&r, &u, &f);
    set_mat(out,
    r.x, r.y, r.z, 0.f,
    u.x, u.y, u.z, 0.f,
    f.x, f.y, f.z, 0.f,
    t->x,t->y,t->z,1.f);
}

void quick_inv(mat4 *out, mat4 *a)
{
    vec4 _a = {a->m[0][0], a->m[1][0], a->m[2][0]};
    vec4 _b = {a->m[0][1], a->m[1][1], a->m[2][1]};
    vec4 _c = {a->m[0][2], a->m[1][2], a->m[2][2]};
    vec4 _t = {a->m[0][3], a->m[1][3], a->m[2][3]};
    set_mat(out,
    _a.x, _b.x, _c.x, 0.f,
    _a.y, _b.y, _c.y, 0.f,
    _a.z, _b.z, _c.z, 0.f,
    -dot(&_t,&_a), -dot(&_t,&_b), -dot(&_t,&_c), 1.f);
}