struct vec4
{
    float x, y, z, w;
};

struct tri
{
    vec4 v[3];
    float brightness;
    float pad[3];
};

struct mat4
{
    float m[4][4];
};

void add(vec4 *out, vec4 *a, vec4 *b);
void sub(vec4 *out, vec4 *a, vec4 *b);
void mul(vec4 *out, vec4 *a, float b);
void div(vec4 *out, vec4 *a, float b);
void normalize(vec4 *out, vec4 *a);
float dot(vec4 *a, vec4 *b);
void cross(vec4 *out, vec4 *a, vec4 *b);
void set_mat(mat4 *out,
float a1, float a2, float a3, float a4,
float b1, float b2, float b3, float b4,
float c1, float c2, float c3, float c4,
float d1, float d2, float d3, float d4);
void mul_mat_vec(vec4 *out, mat4 *mat, vec4 *vec);
void mul_mat_mat(mat4 *out, mat4 *a, mat4 *b);
void rotx(mat4 *out, float angle);
void roty(mat4 *out, float angle);
void rotz(mat4 *out, float angle);
void translation(mat4 *out, float x, float y, float z);
void quick_inv(mat4 *out, mat4 *a);
void point_at(mat4 *out, vec4 *p, vec4 *t, vec4 *up);