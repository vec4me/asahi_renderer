#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define VIEW_SIZE_X 320
#define VIEW_SIZE_Y 200

typedef struct {
  int x, y;
} vec2;

typedef struct {
  int x, y, z;
} vec3;

#define MULTIPLY(value, factor) (((value)*(factor)) >> 8)
#define DIVIDE(value, n) (((value) << 8)/(n))
#define COLOR(R, G, B) r = (R), g = (G), b = (B)

int isqrt(unsigned n) {
  unsigned f = 0, p = 1 << 30, r = n;
  while (p > r)
    p >>= 2;
  while (p != 0) {
    if (r >= f + p) {
      r -= f + p;
      f += p << 1;
    }
    f >>= 1;
    p >>= 2;
  }
  return (int)f;
}

int dot(const vec3 *a, const vec3 *b) {
  return a->x*b->x + a->y*b->y + a->z*b->z;
}

int norm(const vec3 *v) {
  return isqrt(v->x*v->x + v->y*v->y + v->z*v->z);
}

void unit(vec3 *v) {
  int n = norm(v);
  if (n != 0) {
    v->x = DIVIDE(v->x, n);
    v->y = DIVIDE(v->y, n);
    v->z = DIVIDE(v->z, n);
  }
}

int main(int argc, char **argv) {
  if (argc < 7) {
    fprintf(stderr,
            "Usage: %s cam_x cam_y cam_z heading light_x light_y light_z\n",
            argv[0]);
    return 1;
  }

  int8_t sin[256] = {
      0,    3,    6,    9,    12,   16,   19,   22,   25,   28,   31,   34,
      37,   40,   43,   46,   49,   51,   54,   57,   60,   63,   65,   68,
      71,   73,   76,   78,   81,   83,   85,   88,   90,   92,   94,   96,
      98,   100,  102,  104,  106,  107,  109,  111,  112,  113,  115,  116,
      117,  118,  120,  121,  122,  122,  123,  124,  125,  125,  126,  126,
      126,  127,  127,  127,  127,  127,  127,  127,  126,  126,  126,  125,
      125,  124,  123,  122,  122,  121,  120,  118,  117,  116,  115,  113,
      112,  111,  109,  107,  106,  104,  102,  100,  98,   96,   94,   92,
      90,   88,   85,   83,   81,   78,   76,   73,   71,   68,   65,   63,
      60,   57,   54,   51,   49,   46,   43,   40,   37,   34,   31,   28,
      25,   22,   19,   16,   12,   9,    6,    3,    0,    -3,   -6,   -9,
      -12,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,
      -49,  -51,  -54,  -57,  -60,  -63,  -65,  -68,  -71,  -73,  -76,  -78,
      -81,  -83,  -85,  -88,  -90,  -92,  -94,  -96,  -98,  -100, -102, -104,
      -106, -107, -109, -111, -112, -113, -115, -116, -117, -118, -120, -121,
      -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
      -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122,
      -122, -121, -120, -118, -117, -116, -115, -113, -112, -111, -109, -107,
      -106, -104, -102, -100, -98,  -96,  -94,  -92,  -90,  -88,  -85,  -83,
      -81,  -78,  -76,  -73,  -71,  -68,  -65,  -63,  -60,  -57,  -54,  -51,
      -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,
      -12,  -9,   -6,   -3};

  int cos[256];
  for (int i = 0; i < 256; i++) {
    cos[i] = sin[(i + 65)&255];
  }

  unsigned r = 0, g = 0, b = 0;

  vec3 camera_position = {atoi(argv[1]), atoi(argv[2]), atoi(argv[3])};
  int camera_heading = atoi(argv[4]);
  vec3 light_direction = {atoi(argv[5]), atoi(argv[6]), atoi(argv[7])};
  unit(&light_direction);

  int camera_heading_cos = cos[camera_heading&255];
  int camera_heading_sin = sin[camera_heading&255];

  printf("P6\n%d %d\n255\n", VIEW_SIZE_X, VIEW_SIZE_Y);

  uint8_t *color_buffer = calloc(VIEW_SIZE_X*VIEW_SIZE_Y*3, 1);

  for (int pixel_index = 0; pixel_index < VIEW_SIZE_X*VIEW_SIZE_Y;
       pixel_index++) {
    vec2 pixel_position = {pixel_index%VIEW_SIZE_X,
                           pixel_index/VIEW_SIZE_X};
    vec2 view_offset = {VIEW_SIZE_X - (pixel_position.x << 1),
                        VIEW_SIZE_Y - (pixel_position.y << 1)};
    vec3 pixel_direction = {
        view_offset.x*camera_heading_cos/VIEW_SIZE_Y - camera_heading_sin,
        (view_offset.y << 7)/VIEW_SIZE_Y,
        view_offset.x*camera_heading_sin/VIEW_SIZE_Y + camera_heading_cos};
    unit(&pixel_direction);

    int pixel_distance = pixel_direction.y != 0
                             ? DIVIDE(camera_position.y, pixel_direction.y)
                             : 0;

    vec3 hit = {camera_position.x + pixel_distance*pixel_direction.x,
                camera_position.y + pixel_distance*pixel_direction.y,
                camera_position.z + pixel_distance*pixel_direction.z};

    if (pixel_direction.y > 0) {
      COLOR(188, 0, 45);
      int sky = cos[((cos[((hit.z >> 11)&255)] + (hit.x >> 8)) >> 1&255)] +
                cos[(hit.z/500&255)]/4 + 30;
      if (sky < 0)
        r = g = b = sky;
      else if (dot(&pixel_direction, &light_direction) < 64000) {
        COLOR(128 - 128*pixel_direction.y/255,
              179 - 179*pixel_direction.y/255,
              255 - 76*pixel_direction.y/255);
      }
    }
    else if (pixel_direction.y < 0) {
      if (!(((hit.x >> 13)%7)*((hit.z >> 13)%9) != 0)) {
        COLOR(100, 100, 110);
      }
      else {
        COLOR(60, sin[(hit.x/20&255)]/2 + 55, 0);
        if (g > 200) {
          g = 60;
          b = 120;
        }
      }
    }

    color_buffer[pixel_index*3 + 0] = r;
    color_buffer[pixel_index*3 + 1] = g;
    color_buffer[pixel_index*3 + 2] = b;
  }

  fwrite(color_buffer, 1, VIEW_SIZE_X*VIEW_SIZE_Y*3, stdout);
  free(color_buffer);

  return 0;
}