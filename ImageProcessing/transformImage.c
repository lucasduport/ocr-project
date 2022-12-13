#include "transformImage.h"

void toGrey(Image *image)
{
	uc nb_channels = image->nb_channels;
	if (nb_channels == 1)
		return;
	st width = image->width, height = image->height;
	Image *new_image = newImage(1, width, height);
	st len = width * height;
	uc *new_channel = new_image->channels[0];
	uc *r_p = image->channels[0];
	uc *g_p = image->channels[1];
	uc *b_p = image->channels[2];
	uc *a_p = image->channels[3];
	uc *end = new_channel + len;
	for (uc *p = new_channel; p < end; p++, r_p++, g_p++, b_p++, a_p++)
	{
		if (*a_p)
			*p = (*r_p + *g_p + *b_p) / 3;
		else
			*p = 255;
	}
	for (uc i = 0; i < nb_channels; i++)
		free(image->channels[i]);
	free(image->channels);
	image->channels = new_image->channels;
	image->nb_channels = 1;
	free(new_image);
}

void toRGBA(Image *image)
{
	if (image->nb_channels == 4)
		return;
	st width = image->width, height = image->height;
	Image *new_image = newImage(4, width, height);
	st len = width * height;
	for (uc n = 0; n < 3; n++)
		for (st i = 0; i < len; i++)
			new_image->channels[n][i] = image->channels[0][i];
	for (st i = 0; i < len; i++)
		new_image->channels[4][i] = 255;
	freeImage(image);
	*image = *new_image;
}

void toColor(Image *image, uc r, uc g, uc b)
{
	if (image->nb_channels == 1)
		errx(EXIT_FAILURE, "image should be in RGBA");
	st len = image->width * image->height;
	uc color[3] = {r, g, b};
	for (uc n = 0; n < 3; n++)
	{
		uc *channel = image->channels[n];
		uc val = color[n];
		for (st i = 0; i < len; i++)
			channel[i] = val;
	}
}

void createAlpha(Image *image, int min, int max)
{
	uc nb_channels = image->nb_channels;
	if (nb_channels == 1)
		errx(EXIT_FAILURE, "image should be in RGBA");
	st len = image->width * image->height;
	uc *r_channel = image->channels[0];
	uc *g_channel = image->channels[1];
	uc *b_channel = image->channels[2];
	uc *a_channel = image->channels[3];
	for (st i = 0; i < len; i++)
	{
		int val = (r_channel[i] + g_channel[i] + b_channel[i]) / 3;
		if (val < min || val > max)
			a_channel[i] = 0;
	}
}

uc lerp(uc *pxls, float x, float y, st w)
{
	// x1 <= x <= x2
	// y1 <= y <= y2
	st x1 = x;
	st x2 = x1 + 1;
	st y1 = y;
	st y2 = y1 + 1;
	float w_x1 = x2 - x;
	float w_x2 = x - x1;
	float w_y1 = y2 - y;
	float w_y2 = y - y1;
	float val = 0;
	val += pxls[y1 * w + x1] * w_x1 * w_y1;
	val += pxls[y1 * w + x2] * w_x2 * w_y1;
	val += pxls[y2 * w + x1] * w_x1 * w_y2;
	val += pxls[y2 * w + x2] * w_x2 * w_y2;
	return (uc)(val + 0.5);
}

void resizeImage(Image *image, st new_w, st new_h)
{
	st w = image->width, h = image->height;
	float ratio_w = (float)w / new_w;
	float ratio_h = (float)h / new_h;
	float x, y;
	for (uc i = 0; i < image->nb_channels; i++)
	{
		uc *channel = image->channels[i];
		uc *new_channel = (uc *)malloc(new_w * new_h * sizeof(uc));
		if (channel == NULL)
			errx(EXIT_FAILURE, "malloc failed");
		for (st new_y = 0; new_y < new_h; new_y++)
		{
			y = new_y * ratio_h;
			for (st new_x = 0; new_x < new_w; new_x++)
			{
				x = new_x * ratio_w;
				if (x < 0 || x + 1 >= w || y < 0 || y + 1 >= h)
				{
					new_channel[new_y * new_w + new_x] = 0;
					continue;
				}
				new_channel[new_y * new_w + new_x] = lerp(channel, x, y, w);
			}
		}
		free(channel);
		image->channels[i] = new_channel;
	}
	image->width = new_w;
	image->height = new_h;
}

void autoResize(Image *image, st max_w, st max_h)
{
	if (image->width <= max_w && image->height <= max_h)
		return;
	// if ratio_w > ratio_h, then we resize by width
	float ratio_w = (float)image->width / max_w;
	float ratio_h = (float)image->height / max_h;
	if (ratio_w > ratio_h)
		resizeImage(image, max_w, image->height / ratio_w);
	else
		resizeImage(image, image->width / ratio_h, max_h);
}

Image *extractGrid(Image *image, Quad *quad, st new_w, st new_h)
{
	// p1 : ul, p2 : ur,
	// p3 : dl, p4 : dr
	uc nb_channels = image->nb_channels;
	st w = image->width, h = image->height;
	Image *new_image = newImage(nb_channels, new_w, new_h);
	float input[3] = {0, 0, 1};
	float mat[3][3];
	getTransformMatrix(quad, new_w, new_h, mat);
	float res[3];
	for (uc i = 0; i < nb_channels; i++)
	{
		uc *channel = image->channels[i];
		uc *new_channel = new_image->channels[i];
		float x, y;
		for (st new_y = 0; new_y < new_h; new_y++)
		{
			input[1] = new_y;
			for (st new_x = 0; new_x < new_w; new_x++)
			{
				input[0] = new_x;
				matMul33_31(mat, input, res);
				x = res[0] / res[2];
				y = res[1] / res[2];
				if (x < 0 || x + 1 >= w || y < 0 || y + 1 >= h)
				{
					new_channel[new_y * new_w + new_x] = 0;
					continue;
				}
				new_channel[new_y * new_w + new_x] = lerp(channel, x, y, w);
			}
		}
	}
	return new_image;
}

Image *rotateImage(Image *image, int angle, uc background_color)
{
	uc nb_channels = image->nb_channels;
	int w = image->width, h = image->height;
	// all corners ((0, 0), (w, 0), (0, h), (w, h)) must be in the new image
	float _cos = COS[angle];
	float _sin = SIN[angle];
	int new_w = fabs(w * _cos) + fabs(h * _sin);
	int new_h = fabs(w * _sin) + fabs(h * _cos);
	Image *new_image = newImage(nb_channels, new_w, new_h);
	for (uc i = 0; i < nb_channels; i++)
	{
		uc *channel = image->channels[i];
		uc *new_channel = new_image->channels[i];
		float new_x0, new_y0;
		for (int new_y = 0; new_y < new_h; new_y++)
		{
			new_y0 = new_y - new_h / 2.;
			for (int new_x = 0; new_x < new_w; new_x++)
			{
				new_x0 = new_x - new_w / 2.;
				float x = (new_x0)*_cos - (new_y0)*_sin + w / 2;
				float y = (new_x0)*_sin + (new_y0)*_cos + h / 2;
				if (x < 0 || x + 1 >= w || y < 0 || y + 1 >= h)
				{
					new_channel[new_y * new_w + new_x] = background_color;
					continue;
				}
				new_channel[new_y * new_w + new_x] = lerp(channel, x, y, w);
			}
		}
	}
	return new_image;
}
