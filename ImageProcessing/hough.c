#include <math.h>
#include <stdio.h>
#include "hough.h"
#include "display.h"
#include "filters.h"
#include "param.h"
#include "smoothLine.h"
#include "transformImage.h"

#define RANGE_DEL_R 5
#define RANGE_DEL_THETA 1

#define NB_SEGMENTS 200
#define COORDINATES_ERROR 3 // percentage of the size of the image
#define ANGLE_ERROR 15		// in degrees
#define LENGTH_ERROR 1.2	// max ratio of length

int isVertical(st theta)
{
	return (0 <= theta && theta < 45) || (135 <= theta && theta < 225)
		   || (315 <= theta && theta < 360);
}

void fillR_thetaVertical(Image *image, uc *r_theta, st r_max, st theta)
{
	float _cos = COS[theta], _sin = SIN[theta];
	st w = image->width, h = image->height;
	uc *pixels = image->channels[0];
	int value;
	int nb;
	for (st r = 0; r < r_max; r++)
	{
		value = 0;
		nb = 0;
		for (st y = 0; y < h; y++)
		{
			float x = (r - y * _sin) / _cos;
			if (x >= 0 && x < w)
			{
				value += pixels[y * w + (st)x];
				nb++;
			}
		}
		r_theta[r * 360 + theta] = nb ? value / nb : 0;
	}
}

void fillR_thetaHorizontal(Image *image, uc *r_theta, st r_max, st theta)
{
	float _cos = COS[theta], _sin = SIN[theta];
	st w = image->width, h = image->height;
	uc *pixels = image->channels[0];
	int value;
	int nb;
	for (st r = 0; r < r_max; r++)
	{
		value = 0;
		nb = 0;
		for (st x = 0; x < w; x++)
		{
			float y = (r - x * _cos) / _sin;
			if (y >= 0 && y < h)
			{
				value += pixels[(st)y * w + x];
				nb++;
			}
		}
		r_theta[r * 360 + theta] = nb ? value / nb : 0;
	}
}

void fillR_theta(Image *image, uc *r_theta, st r_max)
{
	int vertical = 1;
	float _cos, _sin;
	for (st theta = 0; theta < 360; theta++)
	{
		// vertical: [0, 45[, [135, 180[, [315, 360[
		// horizontal: [45, 135[, [270, 315[
		if (theta == 180)
		{
			theta = 271;
			vertical = 0;
		}
		if (theta == 45 || theta == 135 || theta == 315)
			vertical = !vertical;
		_cos = cos(theta * PI / 180), _sin = sin(theta * PI / 180);
		(vertical ? fillR_thetaVertical : fillR_thetaHorizontal)(
			image, r_theta, r_max, theta);
	}
	for (st r = 0; r < r_max; r++)
		for (st theta = 180; theta <= 270; theta++)
			r_theta[r * 360 + theta] = 0;
}

void printR_theta(uc *r_theta, st r_max)
{
	// display theta [270 - 360[ + [0 - 180[
	static char *chars = " .'_`^\",:;I-!<>~+i?][}{1()|/"
						 "\\ftjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@";
	for (st r = 0; r < r_max; r += 15)
	{
		for (st theta = 270; theta != 180; theta++)
		{
			if (theta == 360)
				theta = 0;
			printf("%c", chars[r_theta[r * 360 + theta] * 70 / 256]);
		}
		printf("\n");
	}
}

void getVerticalLine(Image *image, st r, st theta, uc *line)
{
	uc *pixels = image->channels[0];
	st width = image->width;
	st height = image->height;
	float _cos = cos(theta * PI / 180);
	float _sin = sin(theta * PI / 180);
	for (st y = 0; y < height; y++)
	{
		float x = (r - y * _sin) / _cos;
		if (x < 0 || x >= width)
			line[y] = 0;
		else
			line[y] = pixels[y * width + (st)x];
	}
}

void getHorizontalLine(Image *image, st r, st theta, uc *line)
{
	uc *pixels = image->channels[0];
	st width = image->width;
	st height = image->height;
	float _cos = cos(theta * PI / 180);
	float _sin = sin(theta * PI / 180);
	for (st x = 0; x < width; x++)
	{
		float y = (r - x * _cos) / _sin;
		if (y < 0 || y >= height)
			line[x] = 0;
		else
			line[x] = pixels[(st)y * width + x];
	}
}

void deleteBest(uc *r_theta, st r_max, st best_r, st best_theta)
{
	st r_min = best_r - RANGE_DEL_R > best_r ? 0 : best_r - RANGE_DEL_R;
	r_max = best_r + RANGE_DEL_R > r_max ? r_max : best_r + RANGE_DEL_R;
	st theta_min = (best_theta + 360 - RANGE_DEL_THETA) % 360;
	st theta_max = (best_theta + RANGE_DEL_THETA) % 360;
	for (st r = r_min; r < r_max; r++)
	{
		for (st theta = theta_min; theta % 360 != theta_max; theta++)
		{
			theta %= 360;
			r_theta[r * 360 + theta] = 0;
		}
	}
}

int stDiffSquare(st a, st b)
{
	return a > b ? pow(a - b, 2) : pow(b - a, 2);
}

Segment *getBestSegment(uc *r_theta, st r_max, Image *image, int *left)
{
	st best_value = 0;
	st best_r = 0;
	st best_theta = 0;
	for (st r = 0; r < r_max; r++)
	{
		for (st theta = 270; theta != 180; theta++)
		{
			if (theta == 360)
				theta = 0;
			if (r_theta[r * 360 + theta] > best_value)
			{
				best_value = r_theta[r * 360 + theta];
				best_r = r;
				best_theta = theta;
			}
		}
	}
	if (best_value == 0)
	{
		*left = 0;
		return NULL;
	}
	*left = 1;
	int vertical = isVertical(best_theta);
	st height = image->height, width = image->width;
	st dim = vertical ? height : width;
	uc line[dim];
	if (vertical)
		getVerticalLine(image, best_r, best_theta, line);
	else
		getHorizontalLine(image, best_r, best_theta, line);
	st i_start, i_end;
	smoothLine(line, best_value, dim, &i_start, &i_end);
	float _cos = cos(best_theta * PI / 180);
	float _sin = sin(best_theta * PI / 180);
	float x1, y1, x2, y2;
	if (vertical)
	{
		y1 = 0;
		x1 = (best_r - y1 * _sin) / _cos;
		y2 = height - 1;
		x2 = (best_r - y2 * _sin) / _cos;
	}
	else
	{
		x1 = 0;
		y1 = (best_r - x1 * _cos) / _sin;
		x2 = width - 1;
		y2 = (best_r - x2 * _cos) / _sin;
	}
	float ratio_start = (float)i_start / dim;
	float ratio_end = (float)i_end / dim;
	float x_start = x1 + ratio_start * (x2 - x1);
	float y_start = y1 + ratio_start * (y2 - y1);
	float x_end = x1 + ratio_end * (x2 - x1);
	float y_end = y1 + ratio_end * (y2 - y1);
	r_theta[best_r * 360 + best_theta] = 0;
	if (x_start < 0 || x_end >= width || y_start < 0 || y_end >= height)
		return NULL;
	st length = sqrt(pow(x_end - x_start, 2) + pow(y_end - y_start, 2));
	if (length < dim / 3)
		return NULL;
	if (length > width * LENGTH_ERROR || length > height * LENGTH_ERROR)
		return NULL;
	deleteBest(r_theta, r_max, best_r, best_theta);
	st _x = x_start, _y = y_start, x_ = x_end, y_ = y_end;
	Segment *segment = newSegment(_x, _y, x_, y_, best_theta, best_r, length);
	return segment;
}

void swapPoints(Segment *segment)
{
	st tmp = segment->x1;
	segment->x1 = segment->x2;
	segment->x2 = tmp;
	tmp = segment->y1;
	segment->y1 = segment->y2;
	segment->y2 = tmp;
}

void printSegment(Segment *segment, st i)
{
	printf("Segment%zu: (%zu, %zu) (%zu, %zu)\n", i, segment->x1, segment->y1,
		segment->x2, segment->y2);
}

Point *getIntersection(Segment *segment1, Segment *segment2)
{
	float c1 = cos(segment1->theta * PI / 180),
		  s1 = sin(segment1->theta * PI / 180);
	float c2 = cos(segment2->theta * PI / 180),
		  s2 = sin(segment2->theta * PI / 180);
	int r1 = segment1->r, r2 = segment2->r;
	float det = c1 * s2 - c2 * s1;
	st x = (r1 * s2 - r2 * s1) / det;
	st y = (r2 * c1 - r1 * c2) / det;
	Point *point = newPoint(x, y);
	return point;
}

Point *getTopLeft(Point *p1, Point *p2, Point *p3, Point *p4)
{
	int sum1 = p1->x + p1->y;
	int sum2 = p2->x + p2->y;
	int sum3 = p3->x + p3->y;
	int sum4 = p4->x + p4->y;
	if (sum1 <= sum2 && sum1 <= sum3 && sum1 <= sum4)
		return p1;
	if (sum2 <= sum1 && sum2 <= sum3 && sum2 <= sum4)
		return p2;
	if (sum3 <= sum1 && sum3 <= sum2 && sum3 <= sum4)
		return p3;
	if (sum4 <= sum1 && sum4 <= sum2 && sum4 <= sum3)
		return p4;
	return NULL;
}

Point *getTopRight(Point *p1, Point *p2, Point *p3, Point *p4)
{
	int sum1 = p1->x - p1->y;
	int sum2 = p2->x - p2->y;
	int sum3 = p3->x - p3->y;
	int sum4 = p4->x - p4->y;
	if (sum2 >= sum1 && sum2 >= sum3 && sum2 >= sum4)
		return p2;
	if (sum3 >= sum1 && sum3 >= sum2 && sum3 >= sum4)
		return p3;
	if (sum4 >= sum1 && sum4 >= sum2 && sum4 >= sum3)
		return p4;
	if (sum1 >= sum2 && sum1 >= sum3 && sum1 >= sum4)
		return p1;
	return NULL;
}

Point *getBottomLeft(Point *p1, Point *p2, Point *p3, Point *p4)
{
	int sum1 = p1->x - p1->y;
	int sum2 = p2->x - p2->y;
	int sum3 = p3->x - p3->y;
	int sum4 = p4->x - p4->y;
	if (sum3 <= sum1 && sum3 <= sum2 && sum3 <= sum4)
		return p3;
	if (sum4 <= sum1 && sum4 <= sum2 && sum4 <= sum3)
		return p4;
	if (sum1 <= sum2 && sum1 <= sum3 && sum1 <= sum4)
		return p1;
	if (sum2 <= sum1 && sum2 <= sum3 && sum2 <= sum4)
		return p2;
	return NULL;
}

Point *getBottomRight(Point *p1, Point *p2, Point *p3, Point *p4)
{
	int sum1 = p1->x + p1->y;
	int sum2 = p2->x + p2->y;
	int sum3 = p3->x + p3->y;
	int sum4 = p4->x + p4->y;
	if (sum4 >= sum1 && sum4 >= sum2 && sum4 >= sum3)
		return p4;
	if (sum1 >= sum2 && sum1 >= sum3 && sum1 >= sum4)
		return p1;
	if (sum2 >= sum1 && sum2 >= sum3 && sum2 >= sum4)
		return p2;
	if (sum3 >= sum1 && sum3 >= sum2 && sum3 >= sum4)
		return p3;
	return NULL;
}

int checkAngle(Segment *segment1, Segment *segment2)
{
	int angle1 = segment1->theta;
	int angle2 = segment2->theta;
	int diff = abs(angle1 - angle2 + 90) % 180;
	return diff <= ANGLE_ERROR || diff >= 180 - ANGLE_ERROR;
}

int checkLength(Segment *segment1, Segment *segment2)
{
	int length1 = segment1->length;
	int length2 = segment2->length;
	if (length1 * LENGTH_ERROR < length2)
		return 0;
	if (length2 * LENGTH_ERROR < length1)
		return 0;
	return 1;
}

int checkCoordinates(Segment *s1, Segment *s2, int p2, int min_dist, int *swap)
{
	int diff1, diff2;
	if (!p2)
	{
		diff1 = stDiffSquare(s1->x1, s2->x1) + stDiffSquare(s1->y1, s2->y1);
		diff2 = stDiffSquare(s1->x1, s2->x2) + stDiffSquare(s1->y1, s2->y2);
	}
	else
	{
		diff1 = stDiffSquare(s1->x2, s2->x1) + stDiffSquare(s1->y2, s2->y1);
		diff2 = stDiffSquare(s1->x2, s2->x2) + stDiffSquare(s1->y2, s2->y2);
	}
	if (diff1 <= min_dist)
	{
		*swap = 0;
		return 1;
	}
	if (diff2 <= min_dist)
	{
		*swap = 1;
		return 1;
	}
	return 0;
}

Quad *constructGrid(Segment **segments, st nb_segments, int min_dist)
{
	st i1 = nb_segments - 1;
	Segment *segment1 = segments[i1];
	st segments2[nb_segments];
	int swap;
	st j = 0;
	for (st i2 = 0; i2 < nb_segments; i2++)
	{
		if (i2 == i1)
			continue;
		Segment *segment2 = segments[i2];
		if (!checkAngle(segment1, segment2))
			continue;
		if (!checkLength(segment1, segment2))
			continue;
		if (!checkCoordinates(segment1, segment2, 0, min_dist, &swap))
			continue;
		if (swap)
			swapPoints(segment2);
		segments2[j++] = i2;
		/*
		printf("cmp1-2 : (%zu, %zu) (%zu, %zu)\n", segment1->x1,
		segment1->y1, segment2->x1, segment2->y1);
		printSegment(segment1, 1);
		printSegment(segment2, 2);
		*/
	}
	segments2[j] = nb_segments;

	j = 0;
	st segments3[nb_segments];
	for (st i3 = 0; i3 < nb_segments; i3++)
	{
		if (i3 == i1)
			continue;
		Segment *segment3 = segments[i3];
		if (!checkAngle(segment1, segment3))
			continue;
		if (!checkLength(segment1, segment3))
			continue;
		if (!checkCoordinates(segment1, segment3, 1, min_dist, &swap))
			continue;
		if (swap)
			swapPoints(segment3);
		segments3[j++] = i3;
		/*
		printf("cmp1-3 : (%zu, %zu) (%zu, %zu)\n", segment1->x1,
		segment1->y1, segment3->x1, segment3->y1);
		printSegment(segment1, 1);
		printSegment(segment3, 3);
		*/
	}
	segments3[j] = nb_segments;

	for (st i4 = 0; i4 < nb_segments; i4++)
	{
		if (i4 == i1)
			continue;
		st k;
		for (k = 0; segments2[k] != nb_segments; k++)
		{
			if (i4 == segments2[k])
				break;
		}
		if (segments2[k] != nb_segments)
			continue;
		for (k = 0; segments3[k] != nb_segments; k++)
		{
			if (i4 == segments3[k])
				break;
		}
		if (segments3[k] != nb_segments)
			continue;
		Segment *segment4 = segments[i4];
		for (st i2 = 0; segments2[i2] != nb_segments; i2++)
		{
			Segment *segment2 = segments[segments2[i2]];
			if (!checkAngle(segment2, segment4))
				continue;
			if (!checkLength(segment2, segment4))
				continue;
			if (!checkCoordinates(segment2, segment4, 1, min_dist, &swap))
				continue;
			if (swap)
				swapPoints(segment4);
			/*
			printf("cmp2-4 : (%zu, %zu) (%zu, %zu)\n", segment2->x2,
			segment2->y2, segment4->x1, segment4->y1);
			printSegment(segment2, 2);
			printSegment(segment4, 4);
			*/
			for (st i3 = 0; segments3[i3] != nb_segments; i3++)
			{
				Segment *segment3 = segments[segments3[i3]];
				if (!checkAngle(segment3, segment4))
					continue;
				if (!checkLength(segment3, segment4))
					continue;
				if (!checkCoordinates(segment3, segment4, 1, min_dist, &swap))
					continue;
				if (!swap)
					continue;
				/*
				printf("cmp3-4 : (%zu, %zu) (%zu, %zu)\n", segment3->x2,
				segment3->y2, segment4->x2, segment4->y2);
				printSegment(segment1, 1);
				printSegment(segment2, 2);
				printSegment(segment3, 3);
				printSegment(segment4, 4);
				*/
				Point *p1 = getIntersection(segment1, segment2); // top left
				Point *p2 = getIntersection(segment2, segment4); // top right
				Point *p3 = getIntersection(segment1, segment3); // bottom left
				Point *p4
					= getIntersection(segment3, segment4); // bottom right
				Point *top_left = getTopLeft(p1, p2, p3, p4);
				Point *top_right = getTopRight(p1, p2, p3, p4);
				Point *bottom_left = getBottomLeft(p1, p2, p3, p4);
				Point *bottom_right = getBottomRight(p1, p2, p3, p4);
				Quad *quad
					= newQuad(top_left, top_right, bottom_left, bottom_right);
				return quad;
			}
		}
	}
	return NULL;
}

Quad *detectGrid(Image *image)
{
	st width = image->width, height = image->height;
	st r_max = sqrt(width * width + height * height);
	uc r_theta[r_max * 360];
	fillR_theta(image, r_theta, r_max);
	Segment *segments[NB_SEGMENTS];
	st nb_segments = 0;
	int min_dist = pow(COORDINATES_ERROR / 100.0 * r_max, 2);
	Quad *quad = NULL;
	int left;
	while (nb_segments < NB_SEGMENTS)
	{
		Segment *segment = getBestSegment(r_theta, r_max, image, &left);
		if (!left)
			break;
		// printf("nb_segments : %zu\n", nb_segments);
		if (!segment)
			continue;
		segments[nb_segments] = segment;
		nb_segments++;
		quad = constructGrid(segments, nb_segments, min_dist);
		if (quad)
			break;
	}
	if (!getUIMode())
		showLines(image, segments, nb_segments, 255, 0, 0, 1);
	freeSegments(segments, nb_segments);
	return quad;
}
