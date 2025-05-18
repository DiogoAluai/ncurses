#include<ncurses.h>
#include<stdlib.h>
#include<time.h>

#define MAX_SUPPORTED_X 500
#define MAX_SUPPORTED_Y 200

char **allocate_screen() {
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


void sleep_a_bit() {
	struct timespec sleep_time = {
	    .tv_sec = 0,
	    .tv_nsec = 100000000 // 0.1 seconds 
	};
	nanosleep(&sleep_time, NULL);
}

void draw_matrix_to_window(WINDOW *win, char **matrix, int loopI) {
    for (int y = 0; y < MAX_SUPPORTED_Y; ++y) {
        for (int x = 0; x < MAX_SUPPORTED_X; ++x) {
        	int customColor = 10 + loopI + x + y; // has to be one per color, as changing it in the future fucks up already printed chars' coloring
        	int customPair = customColor;
        	init_color(customColor, 1 + loopI + y*5, 999 - loopI - x*2, 755);
            init_pair(customPair, customColor, -1);
            attron(COLOR_PAIR(customPair));
            mvwaddch(win, y, x, matrix[y][x]);
            attroff(COLOR_PAIR(customPair));
        }
    }
}
void paint(WINDOW *win, char **screen, int xMin, int xMax, int yMin, int yMax, int loopI) {
	int startRow = yMax / 2 - 3;
	int endRow = yMax / 2 + 3;

	//clear
    for (int y = 0; y < MAX_SUPPORTED_Y; ++y)
	    for (int x = 0; x < MAX_SUPPORTED_X; ++x) 
	    	screen[y][x] = ' ';

	// ribbon
	for (int y = startRow; y <= endRow; y++)
	    for (int x = 0; x <= xMax; ++x) 
	    	screen[y][x] = '#';

	draw_matrix_to_window(win, screen, loopI);

}

void main() {
	initscr();
	start_color();
	use_default_colors();

	refresh();

	if(!can_change_color()) {
		printw("Terminal cannot change colors, press any key to exit...");
		getch();
		exit(0);
	}


	int xMin, xMax, yMin, yMax;


	char **screen = allocate_screen();

	int loopI = 0;
	while (1) {		
		getbegyx(stdscr, yMin, xMin);
		getmaxyx(stdscr, yMax, xMax);

		paint(stdscr, screen, xMin, xMax, yMin, yMax, loopI);
		
		loopI++;
		refresh();
		sleep_a_bit();
	}
	
	endwin();
}



















