#include<ncurses.h>
#include<limits.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>

#define FPS 120
#define FRAME_TIME_NS (1000000000L / FPS) // Frame time in nanoseconds
#define MAX_SUPPORTED_X 500
#define MAX_SUPPORTED_Y 200


#define ALPHA_ANGLE 2 // degrees for each step in matrix
#define EMPTY_SPACE_CHARACTER ' '
#define LOOKING_SENSITIVITY 4 //degrees


FILE *logfile;
double hax = 0, hay = 0, haz = 1; // horizontal rotation axis
double vax = 1, vay = 0, vaz = 0; // vertical rotation axis	


void validate() {
	if (MAX_SUPPORTED_Y <= 0 || MAX_SUPPORTED_X <= 0) {
		printf("Thats silly ain it?\n");
		exit(1);
	}
}

void throttled_refresh(WINDOW *win, struct timespec loopStart, struct timespec loopEnd) {
    long elapsed_ns = (loopEnd.tv_sec - loopStart.tv_sec) * 1000000000L +
                  (loopEnd.tv_nsec - loopStart.tv_nsec);

	// throttle logic
    if (elapsed_ns < FRAME_TIME_NS) {
        struct timespec sleep_time = {
            .tv_sec = 0,
            .tv_nsec = FRAME_TIME_NS - elapsed_ns
        };
        nanosleep(&sleep_time, NULL);
    }    
    wrefresh(win);
}


char **allocate_screen(void) {
    char **matrix = malloc(MAX_SUPPORTED_Y * sizeof(char *));
    if (!matrix) return NULL;

    for (int y = 0; y < MAX_SUPPORTED_Y; ++y) {
        matrix[y] = malloc(MAX_SUPPORTED_X * sizeof(char));
	    for (int x = 0; x < MAX_SUPPORTED_X; ++x) matrix[y][x] = EMPTY_SPACE_CHARACTER;
        if (!matrix[y]) {
            // cleanup in case of failure
            for (int i = 0; i < y; ++i) free(matrix[i]);
            free(matrix);
            return NULL;
        }
    }
    return matrix;
}

void draw_matrix_to_window(WINDOW *win, char **matrix) {
    for (int y = 0; y < MAX_SUPPORTED_Y; ++y) {
        for (int x = 0; x < MAX_SUPPORTED_X; ++x) {
            mvwaddch(win, y, x, matrix[y][x]);
        }
    }
}

int min6(int a, int b, int c, int d, int e, int f) {
    int min = a;
    if (b < min) min = b;
    if (c < min) min = c;
    if (d < min) min = d;
    if (e < min) min = e;
    if (f < min) min = f;
    return min;
}

double distance3d(double x1, double y1, double z1, double x2, double y2, double z2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dz = z2 - z1;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

// Rotate vector (vx, vy, vz) around unit axis (kx, ky, kz) by angle theta (in radians)
void rodrigues_rotation(
    double vx, double vy, double vz,
    double kx, double ky, double kz,
    double degrees,
    double *rx, double *ry, double *rz) {

    double theta = degrees * M_PI / 180.0;
    double cost = cos(theta);
    double sint = sin(theta);

    // Cross product k × v
    double cx = ky * vz - kz * vy;
    double cy = kz * vx - kx * vz;
    double cz = kx * vy - ky * vx;

    // Dot product k ⋅ v
    double dot = kx * vx + ky * vy + kz * vz;

    // Rodrigues' formula
    *rx = vx * cost + cx * sint + kx * dot * (1 - cost);
    *ry = vy * cost + cy * sint + ky * dot * (1 - cost);
    *rz = vz * cost + cz * sint + kz * dot * (1 - cost);
}

double does_it_hit_y_0(double fx, double fy, double fz, double lx, double ly, double lz) {
	double k = (fy * (-1)) / ly;

	double cy = 0; // plane: y = 0
	double cx = fx + k * lx; 
	double cz = fz + k * lz; 

	bool hit = round(cz) >= 0 && round(cz) <= 100 && round(cx) >= 0 && round(cx) <= 100;
	if(!hit) return INT_MAX;
	return distance3d(fx,fy,fz, cx,cy,cz);
}

// 		fy + k*ly = 100
//  <=> k = (100 - fy) / ly
double does_it_hit_y_100(double fx, double fy, double fz, double lx, double ly, double lz) {
	double k = (100.0 - fy) / ly;

	double cy = 100; // plane: y = 100
	double cx = fx + k * lx; 
	double cz = fz + k * lz; 
	
	bool hit = round(cz) >= 0 && round(cz) <= 100 && round(cx) >= 0 && round(cx) <= 100;	
	if(!hit) return INT_MAX;
	return distance3d(fx,fy,fz, cx,cy,cz);
}


double does_it_hit_x_0(double fx, double fy, double fz, double lx, double ly, double lz) {
	double k = (fx * (-1)) / lx;

	double cx = 0; // plane: x = 0
	double cy = fy + k * ly; 
	double cz = fz + k * lz; 
	
	bool hit = round(cz) >= 0 && round(cz) <= 100 && round(cy) >= 0 && round(cy) <= 100;	
	if(!hit) return INT_MAX;
	return distance3d(fx,fy,fz, cx,cy,cz);
}

double does_it_hit_x_100(double fx, double fy, double fz, double lx, double ly, double lz) {
	double k = (100.0 - fx) / lx;

	double cx = 100; // plane: x = 100
	double cy = fy + k * ly; 
	double cz = fz + k * lz; 
	
	bool hit = round(cz) >= 0 && round(cz) <= 100 && round(cy) >= 0 && round(cy) <= 100;	
	if(!hit) return INT_MAX;
	return distance3d(fx,fy,fz, cx,cy,cz);
}


double does_it_hit_z_0(double fx, double fy, double fz, double lx, double ly, double lz) {
	double k = (fz * (-1)) / lz;

	double cx = fx + k * lx; 
	double cy = fy + k * ly; 
	double cz = 0; // plane: z = 0
	
	bool hit = round(cy) >= 0 && round(cy) <= 100 && round(cx) >= 0 && round(cx) <= 100;	
	if(!hit) return INT_MAX;
	return distance3d(fx,fy,fz, cx,cy,cz);
}

double does_it_hit_z_100(double fx, double fy, double fz, double lx, double ly, double lz) {
	double k = (100.0 - fz) / lz;

	double cx = fx + k * lx; 
	double cy = fy + k * ly; 
	double cz = 100; // plane: z = 100
	
	bool hit = round(cy) >= 0 && round(cy) <= 100 && round(cx) >= 0 && round(cx) <= 100;	
	if(!hit) return INT_MAX;
	return distance3d(fx,fy,fz, cx,cy,cz);
}


double does_it_hit_the_box(double fx, double fy, double fz, double lx, double ly, double lz) {
	double hits_x_0     = does_it_hit_x_0  (fx, fy, fz, lx, ly, lz);
	double hits_x_100   = does_it_hit_x_100(fx, fy, fz, lx, ly, lz);
	double hits_y_0     = does_it_hit_y_0  (fx, fy, fz, lx, ly, lz);
	double hits_y_100   = does_it_hit_y_100(fx, fy, fz, lx, ly, lz);
	double hits_z_0     = does_it_hit_z_0  (fx, fy, fz, lx, ly, lz);
	double hits_z_100   = does_it_hit_z_100(fx, fy, fz, lx, ly, lz);
	
	return min6(hits_x_0, hits_x_100, hits_y_0, hits_y_100, hits_z_0, hits_z_100);
}

char determine_light_level(double value) {
    if (value >= 120.0) {
        return '.';
    } else if (value >= 100.0) {
        return ':';
    } else if (value >= 80.0) {
        return '*';
    } else if (value >= 60.0) {
        return '=';
    } else {
        return '#';
    }
}

void ray_cast_with_angle(char** screen, double vrangle, double hrangle, int y, int x,
		double fx, double fy, double fz, double lx, double ly, double lz) {

	double rx, ry, rz;
    rodrigues_rotation(lx, ly, lz, vax, vay, vaz, vrangle, &rx, &ry, &rz);
    rodrigues_rotation(rx, ry, rz, hax, hay, haz, hrangle, &rx, &ry, &rz);

    double distance_to_hit = does_it_hit_the_box(fx, fy, fz, rx, ry, rz);
    bool hits_box = distance_to_hit != INT_MAX;
	if (hits_box) {
		fprintf(logfile, "Hit with distance: %lf\n", distance_to_hit);
		char light_aware_chare = determine_light_level(distance_to_hit);
		screen[y][x] = light_aware_chare;
	}
	else {
		screen[y][x] = EMPTY_SPACE_CHARACTER;
	}
}


void ray_cast_box(char **screen, int xMin, int xMax, int yMin, int yMax, 
		double fx, double fy, double fz, double lx, double ly, double lz) {
	// Existing box (in our minds) goes from (0,0,0) to (100,100,100) in 3D space
	// No need to keep it all in memory, as we can define it by six 3D planes:
	//	- x = 0
	//	- x = 100
	//	- y = 0
	//	- y = 100
	//	- z = 0
	//	- z = 100
	// Naturally, all of them have the restriction that the other 2 coordinates must be in [0, 100]

	int yMid = yMax / 2; //yMin is always 0
	int xMid = xMax / 2; //xMin is always 0

	// (yMid, xMid) represents the point where the camera hits looking straight ahead, i.e. no angle.
	// (50, 0, 50)


	bool hits = does_it_hit_the_box(fx, fy, fz, lx, ly, lz);
	screen[yMid][xMid] = hits ? '#' : EMPTY_SPACE_CHARACTER;
	
	double vrangle = 0;
	for (int i=0; yMid + i < yMax && vrangle < 45; i++) {
		vrangle = i * ALPHA_ANGLE;
		double hrangle = 0;
		for (int j=0; xMid + j < xMax && hrangle < 45; j++) {
			hrangle = j * ALPHA_ANGLE;
			ray_cast_with_angle(screen, vrangle, hrangle, yMid + i, xMid + j,
				fx, fy, fz, lx, ly, lz);		
			ray_cast_with_angle(screen, -vrangle, hrangle, yMid - i, xMid + j,
				fx, fy, fz, lx, ly, lz);
			ray_cast_with_angle(screen, vrangle, -hrangle, yMid + i, xMid - j,
				fx, fy, fz, lx, ly, lz);
			ray_cast_with_angle(screen, -vrangle, -hrangle, yMid - i, xMid - j,
				fx, fy, fz, lx, ly, lz);
		}
	}
}

void handle_input(double* fx, double* fy, double* fz, double* lx, double* ly, double* lz) {
	int ch = getch(); // blocker

	double rx, ry, rz;
	switch(ch) {
		case KEY_UP:
			rodrigues_rotation(*lx, *ly, *lz, vax, vay, vaz, -LOOKING_SENSITIVITY*0.5, lx, ly, lz);
			break;
		case KEY_DOWN:
			rodrigues_rotation(*lx, *ly, *lz, vax, vay, vaz, LOOKING_SENSITIVITY*0.5, lx, ly, lz);
			break;
		case KEY_LEFT:
			rodrigues_rotation(*lx, *ly, *lz, hax, hay, haz, -LOOKING_SENSITIVITY, lx, ly, lz);
			break;
		case KEY_RIGHT:
			rodrigues_rotation(*lx, *ly, *lz, hax, hay, haz, LOOKING_SENSITIVITY, lx, ly, lz);
			break;
		case 'a':
			rodrigues_rotation(*lx, *ly, *lz, hax, hay, haz, -90, &rx, &ry, &rz);
			*fy = *fy + ry*2;
			*fx = *fx + rx*2;
			break;
		case 'd':
			rodrigues_rotation(*lx, *ly, *lz, hax, hay, haz, 90, &rx, &ry, &rz);
			*fy = *fy + ry*2;
			*fx = *fx + rx*2;
			break;
		case 'w':
			*fy = *fy + *ly;
			*fx = *fx + *lx;
			break;
		case 's':
			*fy = *fy - *ly;
			*fx = *fx - *lx;
			break;
		case 'Q':
		case 'q':
			fclose(logfile);
			endwin();
			exit(0);
		break;
	}
}

void print_commands() {
	mvprintw(0, 0, "To move use WASD\n");
	printw("To look around use arrow keys\n");
	printw("Press Q to leave\n");
}

void main() {
	validate();
	initscr();
	logfile = fopen("debug.log", "w");
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/
    curs_set(FALSE);      // Hide the cursor
    noecho();
	struct timespec start, end;

	int xMin, xMax, yMin, yMax;
	char **screen = allocate_screen();
	
	// Camera point, the F point, is (50, -100, 50)
	double fx = 50, fy = -100, fz = 50;
	// looking vector
	double lx = 0, ly = 1, lz = 0;

	while (1) {
        clock_gettime(CLOCK_MONOTONIC, &start);

		getbegyx(stdscr, yMin, xMin);
		getmaxyx(stdscr, yMax, xMax);

        clock_gettime(CLOCK_MONOTONIC, &end);

        ray_cast_box(screen, xMin, xMax, yMin, yMax, fx, fy, fz, lx, ly, lz);

        draw_matrix_to_window(stdscr, screen);
        
		print_commands();
		throttled_refresh(stdscr, start, end);
		handle_input(&fx, &fy, &fz, &lx, &ly, &lz);
	}

	fclose(logfile);
	endwin();
}










