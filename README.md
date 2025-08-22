## Notes
This is a practice repository for the ncurses library.

#### Some links
- https://dev.to/tbhaxor/playing-with-ncurses-cursor-part-2-1gil
- tutorial playlist: https://www.youtube.com/watch?v=lV-OPQhPvSM&list=PL2U2TQ__OrQ8jTf0_noNKtHMuYlyxQl4v
- Keys: https://www.gnu.org/software/guile-ncurses/manual/html_node/Getting-characters-from-the-keyboard.html

#### Challenges
- [X] char rain (https://labex.io/courses/project-creating-a-code-rain-in-c-using-ncurses)
- [X] box increasing and decrising size, with inputs
- [X] rainbow or something
- [x] donut-like simulation. Let's do a box 

#### How to run
Build with gcc:
```bash
gcc 1_rain.c -lncurses # link to ncurses must be at the end
#gcc 4_simulate_box.c -lncurses -lm # build simulation with math link to
```
Then simply run the executable with `./a.out`.
