#include "matrices.h"

void invMat33(float mat[3][3], float res[3][3])
{
	float a = mat[0][0], b = mat[0][1], c = mat[0][2];
	float d = mat[1][0], e = mat[1][1], f = mat[1][2];
	float g = mat[2][0], h = mat[2][1], i = mat[2][2];
	float det = a * e * i + b * f * g + c * d * h - c * e * g - b * d * i
				- a * f * h;
	res[0][0] = (e * i - f * h) / det, res[0][1] = (c * h - b * i) / det,
	res[0][2] = (b * f - c * e) / det;
	res[1][0] = (f * g - d * i) / det, res[1][1] = (a * i - c * g) / det,
	res[1][2] = (c * d - a * f) / det;
	res[2][0] = (d * h - e * g) / det, res[2][1] = (b * g - a * h) / det,
	res[2][2] = (a * e - b * d) / det;
}

void matMul33_31(float mat33[3][3], float mat31[3], float res[3])
{
	float a = mat33[0][0], b = mat33[0][1], c = mat33[0][2];
	float d = mat33[1][0], e = mat33[1][1], f = mat33[1][2];
	float g = mat33[2][0], h = mat33[2][1], i = mat33[2][2];
	float x = mat31[0], y = mat31[1], z = mat31[2];
	res[0] = a * x + b * y + c * z;
	res[1] = d * x + e * y + f * z;
	res[2] = g * x + h * y + i * z;
}

void matMul33_33(float mat1[3][3], float mat2[3][3], float res[3][3])
{
	float a = mat1[0][0], b = mat1[0][1], c = mat1[0][2];
	float d = mat1[1][0], e = mat1[1][1], f = mat1[1][2];
	float g = mat1[2][0], h = mat1[2][1], i = mat1[2][2];
	float j = mat2[0][0], k = mat2[0][1], l = mat2[0][2];
	float m = mat2[1][0], n = mat2[1][1], o = mat2[1][2];
	float p = mat2[2][0], q = mat2[2][1], r = mat2[2][2];
	res[0][0] = a * j + b * m + c * p, res[0][1] = a * k + b * n + c * q,
	res[0][2] = a * l + b * o + c * r;
	res[1][0] = d * j + e * m + f * p, res[1][1] = d * k + e * n + f * q,
	res[1][2] = d * l + e * o + f * r;
	res[2][0] = g * j + h * m + i * p, res[2][1] = g * k + h * n + i * q,
	res[2][2] = g * l + h * o + i * r;
}

void getMatrixFromCorners(Quad *quad, float res[3][3])
{
	float x1 = quad->p1->x, y1 = quad->p1->y;
	float x2 = quad->p2->x, y2 = quad->p2->y;
	float x3 = quad->p3->x, y3 = quad->p3->y;
	float x4 = quad->p4->x, y4 = quad->p4->y;
	float mat[3][3] = {{x1, x2, x4}, {y1, y2, y4}, {1, 1, 1}};
	float mat_[3] = {x3, y3, 1};
	float inv_mat[3][3];
	invMat33(mat, inv_mat);
	float lmt[3];
	matMul33_31(inv_mat, mat_, lmt);
	for (st i = 0; i < 3; i++)
		for (st j = 0; j < 3; j++)
			res[j][i] = mat[j][i] * lmt[i];
}

void getTransformMatrix(Quad *quad, st new_w, st new_h, float res[3][3])
{
	Point *p1 = newPoint(0, 0);
	Point *p2 = newPoint(new_w, 0);
	Point *p3 = newPoint(0, new_h);
	Point *p4 = newPoint(new_w, new_h);
	Quad *new_quad = newQuad(p1, p2, p3, p4);
	float mat_a[3][3];
	getMatrixFromCorners(quad, mat_a);
	float mat_b[3][3];
	getMatrixFromCorners(new_quad, mat_b);
	float inv_b[3][3];
	invMat33(mat_b, inv_b);
	matMul33_33(mat_a, inv_b, res);
	freeQuad(new_quad);
}
