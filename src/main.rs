use std::env;
use std::io::{self, Write};

const VIEW_SIZE_X: usize = 320;
const VIEW_SIZE_Y: usize = 200;

#[derive(Copy, Clone)]
struct Vec2 {
	x: i32,
	y: i32,
}

#[derive(Copy, Clone)]
struct Vec3 {
	x: i32,
	y: i32,
	z: i32,
}

fn sqrt(n: u32) -> i32 {
	let mut f = 0u32;
	let mut p = 1u32 << 30;
	let mut r = n;

	while p > r {
		p >>= 2;
	}

	while p != 0 {
		if r >= f + p {
			r -= f + p;
			f += p << 1;
		}
		f >>= 1;
		p >>= 2;
	}

	f as i32
}

fn dot3(a: &Vec3, b: &Vec3) -> i32 {
	a.x*b.x + a.y*b.y + a.z*b.z
}

fn norm3(v: &Vec3) -> i32 {
	sqrt((v.x*v.x + v.y*v.y + v.z*v.z) as u32)
}

macro_rules! multiply {
	($value:expr, $factor:expr) => {
		($value * $factor) >> 8
	};
}

macro_rules! divide {
	($value:expr, $n:expr) => {
		($value << 8) / $n
	};
}

fn unit3(v: &mut Vec3) {
	let n = norm3(v);
	v.x = divide!(v.x, n);
	v.y = divide!(v.y, n);
	v.z = divide!(v.z, n);
}

fn main() {
	let sin: [i8; 256] = [
		0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81,
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
		-25, -22, -19, -16, -12, -9, -6, -3,
	];

	let mut cos = [0; 256];
	for i in 0..256 {
		cos[i] = sin[(i + 65) & 255] as i32;
	}

	let args: Vec<i32> = env::args().skip(1).map(|arg| arg.parse().unwrap()).collect();
	let camera_position = Vec3 {
		x: args[0],
		y: args[1],
		z: args[2],
	};
	// this shouldnt really be mutable
	let mut light_direction = Vec3 {
		x: args[4],
		y: args[5],
		z: args[6],
	};
	unit3(&mut light_direction);

	let camera_heading = args[3];
	let camera_heading_cos = cos[(camera_heading & 255) as usize];
	let camera_heading_sin = sin[(camera_heading & 255) as usize] as i32;

	println!("P6\n{} {}\n255", VIEW_SIZE_X, VIEW_SIZE_Y);

	let mut color_buffer: Vec<u8> = vec![0; VIEW_SIZE_X * VIEW_SIZE_Y * 3];

	for pixel_index in 0..VIEW_SIZE_X * VIEW_SIZE_Y {
		macro_rules! set_r {
		    ($value:expr) => {
		        color_buffer[pixel_index * 3 + 0] = $value as u8;
		    };
		}

		macro_rules! set_g {
		    ($value:expr) => {
		        color_buffer[pixel_index * 3 + 1] = $value as u8;
		    };
		}

		macro_rules! set_b {
		    ($value:expr) => {
		        color_buffer[pixel_index * 3 + 2] = $value as u8;
		    };
		}

		macro_rules! get_r {
		    () => {
		        color_buffer[pixel_index * 3 + 0]
		    };
		}

		macro_rules! get_g {
		    () => {
		        color_buffer[pixel_index * 3 + 1]
		    };
		}

		macro_rules! get_b {
		    () => {
		        color_buffer[pixel_index * 3 + 2]
		    };
		}

		macro_rules! color {
		    ($channel:expr) => {
		        color_buffer[pixel_index * 3 + $channel]
		    };
		}

		let pixel_position = Vec2 {
			x: (pixel_index % VIEW_SIZE_X) as i32,
			y: (pixel_index / VIEW_SIZE_X) as i32,
		};

		let view_offset = Vec2 {
			x: VIEW_SIZE_X as i32 - (pixel_position.x << 1) as i32,
			y: VIEW_SIZE_Y as i32 - (pixel_position.y << 1) as i32,
		};
		let mut pixel_direction = Vec3 {
			x: view_offset.x * camera_heading_cos / VIEW_SIZE_Y as i32 - camera_heading_sin,
			y: (view_offset.y << 7) / VIEW_SIZE_Y as i32,
			z: view_offset.x * camera_heading_sin / VIEW_SIZE_Y as i32 + camera_heading_cos,
		};
		unit3(&mut pixel_direction);

		let pixel_distance = if pixel_direction.y != 0 {
			divide!(150, pixel_direction.y)
		} else {
			0
		};

		let hit = Vec3 {
			x: camera_position.x + pixel_distance * pixel_direction.x,
			y: camera_position.y + pixel_distance * pixel_direction.y,
			z: camera_position.z + pixel_distance * pixel_direction.z,
		};

		if pixel_direction.y > 0 {
			set_r!(188);
			set_g!(0);
			set_b!(45);

			let sky = cos[((cos[((hit.z >> 11) & 255) as usize] + (hit.x >> 8)) >> 1 & 255) as usize]
				+ cos[(hit.z / 500 & 255) as usize] / 4
				+ 30;
			if sky < 0 {
				set_r!(sky);
				set_g!(sky);
				set_b!(sky);
			} else if dot3(&pixel_direction, &light_direction) < 64000 {
				set_r!(128 - 128 * pixel_direction.y / 255);
				set_g!(179 - 179 * pixel_direction.y / 255);
				set_b!(255 - 76 * pixel_direction.y / 255);
			}
		} else if pixel_direction.y < 0 {
			set_r!(77);
			set_g!(40);
			set_b!(0);

			if !(((hit.x >> 13) % 7) * ((hit.z >> 13) % 9) != 0) {
				set_r!(100);
				set_g!(100);
				set_b!(110);
			} else {
				set_r!(60);
				set_g!(sin[(hit.x / 20 & 255) as usize] as i32 / 2 + 55);
				set_b!(0);

				// Checking if it's negative (overflow)
				if get_g!() > 200 {
					set_g!(60);
					set_b!(120);
				}
			}
		}
	}

	io::stdout().write_all(&color_buffer).unwrap();
}
