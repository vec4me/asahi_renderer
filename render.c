#include <stdio.h>
#include <stdlib.h>
enum {X, Y, Z, W};
enum {R, G, B, A};
#define accuracy 10
#define to_float(i) i << accuracy
#define get_multiply(a, b) a*b >> accuracy
#define get_min(a, b) a^(a^b)&-(a > b)
#define get_max(a, b) a^(a^b)&-(a < b)
#define get_lo_word(a) a&0xffff
#define get_hi_word(a) get_lo_word(a >> 16)
#define get_pixel(x, y) color_buffer[x + window_size[X]*y]
#define set_pixel(x, y, r, g, b) get_pixel(x, y) = c
static int get_square_root(unsigned n) {
	register unsigned f, p, r;
	f = 0;
	p = 1 << 30;
	r = n;
	while (p > r) p >>= 2;
	while (p) {
		if (r >= f + p) {
			r = r - f - p;
			f += (p << 1);
		}
		f >>= 1;
		p >>= 2;
	}
	return (int)f;
}
static int get_dot(int *a, int *b) {return a[X]*b[X] + a[Y]*b[Y] + a[Z]*b[Z];}
static int get_square(int *v) {return v[X]*v[X] + v[Y]*v[Y] + v[Z]*v[Z];}
static int get_length(int *v) {return get_square_root(v[X]*v[X] + v[Y]*v[Y] + v[Z]*v[Z]);}
static void unitize(int *v) {
	int n = get_length(v);
	v[X] = to_float(v[X])/n;
	v[Y] = to_float(v[Y])/n;
	v[Z] = to_float(v[Z])/n;
}
//static int sdfblock(int px, int py, int pz, int sx, int sy, int sz) {}
int main(int argc, char **argv) {
	static char sin[] = {0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81,
 83, 85, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116, 117, 118, 120, 121, 122,
 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123,
 122, 122, 121, 120, 118, 117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92, 90, 88,
 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51, 49, 46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9,
 6, 3, 0, -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46, -49, -51, -54, -57, -60, -63,
 -65, -68, -71, -73, -76, -78, -81, -83, -85, -88, -90, -92, -94, -96, -98, -100, -102, -104, -106, -107, -109,
 -111, -112, -113, -115, -116, -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126,
 -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120,
 -118, -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100, -98, -96, -94, -92, -90, -88,
 -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51, -49, -46, -43, -40, -37, -34, -31, -28,
 -25, -22, -19, -16, -12, -9, -6, -3};
	int cos[sizeof sin];
	for (int i = sizeof sin; i--;) cos[i] = sin[(i + 65)&255];
	enum {camera_position_x, camera_position_y, camera_position_z, camera_rotation_y_index, pixel_direction_x, pixel_direction_y, pixel_direction_z};
	int arg[7];
	for (int i = argc; --i;) arg[i - 1] = (int)atoi(argv[i]);
	int camera_position[] = {arg[camera_position_x], arg[camera_position_y], arg[camera_position_z]};
	int pixel_direction[] = {arg[pixel_direction_x], arg[pixel_direction_y], arg[pixel_direction_z]};
	int camera_rotation_y = arg[camera_rotation_y_index];
	int window_size[] = {320, 200};
	char *color_buffer = malloc(3*window_size[X]*window_size[Y]);
	camera_position[Y] = to_float(100);
	for (int f = 2; --f;) {
		int camera_rotation_y_cos = cos[camera_rotation_y&255];
		int camera_rotation_y_sin = sin[camera_rotation_y&255];
		for (int i = window_size[X]*window_size[Y]; i--;) {
			int ix = i%window_size[X];
			int iy = (int)(i/window_size[X]);
			int window_offset[] = {window_size[X] - (ix<<1), (iy<<1) - window_size[Y]};
			int camera_forward[] = {window_offset[X]*camera_rotation_y_cos/window_size[Y] - camera_rotation_y_sin, (window_offset[Y]<<7)/window_size[Y], window_offset[X]*camera_rotation_y_sin/window_size[Y] + camera_rotation_y_cos};
			unitize(&camera_forward);
			int color[] = {0, 0, 0};
			if (camera_forward[Y] > 0) {
				int pixel_distance = 50000/camera_forward[Y];
				int hit[] = {camera_position[X] + pixel_distance*camera_forward[X], camera_position[Z] + pixel_distance*camera_forward[Z]};
				color[R] = 188;
				color[G] = 0;
				color[B] = 45;
				//int sky = cos[hit[X]/1000&255];
				//int sky = cos[(cos[(hit[Z]>>8)&255] + (hit[X]>>8))>>1&255] + cos[hit[Z]/500&255]/2 + 30;
				int sky = 255;
				if (sky < 0) {
					color[R] = sky;
					color[G] = sky;
					color[B] = sky;
				}
				else if (get_dot(&camera_forward, &pixel_direction) < to_float(1)/10) {
					//color[R] = 0;
					//color[G] = 0;
					//color[B] = 255;
					//color[R] = 128 - 128*camera_forward[Y]/255;
					//color[G] = 179 - 179*camera_forward[Y]/255;
					//color[B] = 255 - 76*camera_forward[Y]/255;
				}
				//color[R] = 0;
				//color[G] = 0;
				//color[B] = hit[X]&255;
			}
			else if (camera_forward[Y] < 0) {
				int pixel_distance = camera_position[Y]/-camera_forward[Y];
				int hit[] = {camera_position[X] + pixel_distance*camera_forward[X], camera_position[Z] + pixel_distance*camera_forward[Z]};
				color[R] = 77;
				color[G] = 40;
				color[B] = 0;
				if (!((hit[X]/6000%7))) {
					color[R] = 100;
					color[G] = 100;
					color[B] = 110;
				}
				else {
					color[R] = 60;
					color[G] = sin[hit[X]/20&255]/2 + 55;
					color[B] = 0;
					if (color[G] < 0) {
						color[G] = 60;
						color[B] = 120;
						//color[R] = get_pixel()
						//color[G]
						//color[B]
					}
				}
			}
			color_buffer[3*i + 0] = color[R];
			color_buffer[3*i + 1] = color[G];
			color_buffer[3*i + 2] = color[B];
		}
	}
	printf("P6\n%i %i\n255\n", window_size[X], window_size[Y]);
	fwrite(color_buffer, sizeof(char), 3*window_size[X]*window_size[Y], stdout);
	return 0;
}