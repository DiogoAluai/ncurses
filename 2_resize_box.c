#include<ncurses.h>





WINDOW *create_newwin(int height, int width, int starty, int startx) {	
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);
	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(local_win);
	delwin(local_win);
}

WINDOW *resize_win(WINDOW* win, int height, int width, int starty, int startx) {
	 destroy_win(win);
	 return create_newwin(height, width, starty, startx);
}

void main() {
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/
	curs_set(FALSE);

	printw("Move with: w a s d");
	mvprintw(1, 0, "Resize with arrow keys");
	refresh(); // refresh faz a differença mesmo que nao seja dado print antes, usa esta linha!

	int w=10, h=10, starty=10, startx=10;
	WINDOW *win = create_newwin(h, w, starty, startx);
	wrefresh(win);
	int ch;
	while((ch = getch()) != KEY_F(1)) {
		switch(ch) {
			case KEY_UP:
				w++;
				break;
			case KEY_DOWN:
				w--;
				break;
			case KEY_LEFT:
				h--;
				break;
			case KEY_RIGHT:
				h++;
				break;
			case 'a':
				startx--;
				break;
			case 'd':
				startx++;
				break;
			case 'w':
				starty--;
				break;
			case 's':
				starty++;
				break;
				
		}
		wborder(win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
		wrefresh(win);
		delwin(win);
		win = create_newwin(h, w, starty, startx);	
		wrefresh(win);
	}
	
		
	getch();
	endwin();
}









