#include<ncurses.h>
#include<stdlib.h>
#include<time.h>

#define FPS 120
#define FRAME_TIME_NS (1000000000L / FPS) // Frame time in nanoseconds
#define MAX_SUPPORTED_X 500
#define MAX_SUPPORTED_Y 200


void validate() {
	if (MAX_SUPPORTED_Y <= 0 || MAX_SUPPORTED_X <= 0) {
		printf("Thats silly ain it?\n");
		exit(1);
	}
}

// max exclusive
int rand_int(int max) {
	return rand() % max;
}

char rand_char() {
    const char specials[] = "!@#$%^&*()-_=+[]{};:,.<>?";
    int type = rand() % 3;

    switch (type) {
        case 0: return 'a' + rand_int(26);
        case 1: return 'A' + rand_int(26);
        case 2: return specials[rand_int(sizeof(specials) - 1)];
    }
    return '?';
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
	    for (int x = 0; x < MAX_SUPPORTED_X; ++x) matrix[y][x] = ' ';
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


void make_it_rain(char** screen, int xMin, int xMax, int yMin, int yMax) {
	int newYIndex;
	char droplet;
	// falling droplets
    for (int y = yMax; y >= 0; y--) {
        for (int x = xMax; x >= 0; x--) {
			if (screen[y][x] == ' ') 
				continue;

			droplet = screen[y][x];
			newYIndex = y + 1;
			if (newYIndex < yMax) 
				screen[newYIndex][x] = droplet;

			screen[y][x] = ' ';
        }
    }

	// generate new droplets
	droplet = rand_char();
	int cloudIndex = rand_int(xMax);
	screen[0][cloudIndex] = droplet;
}

void main() {
	validate();
	initscr();
    curs_set(FALSE);      // Hide the cursor
    struct timespec start, end;

	int xMin, xMax, yMin, yMax;
	int x, y;
	char **screen = allocate_screen();
	int i=0;
	
	while (1) {
        clock_gettime(CLOCK_MONOTONIC, &start);

		getbegyx(stdscr, yMin, xMin);
		getmaxyx(stdscr, yMax, xMax);

		make_it_rain(screen, xMin, xMax, yMin, yMax);
		
		draw_matrix_to_window(stdscr, screen);

        clock_gettime(CLOCK_MONOTONIC, &end);
        
		throttled_refresh(stdscr, start, end);
	}

	endwin();
}










