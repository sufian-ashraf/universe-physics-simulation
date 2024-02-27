// #include "iGraphics.h"
#include "object.h"

int body_count = 11;
int index_left = -1;
int str_index = 0;

// Body **bodies = create_solar_system(&body_count);
Body **bodies = create_symmetric_system(body_count);
Button *custom_btn = create_button();
Button *default_btn = create_button();
Button *count_btn = create_button();
int x = 300, y = 300, r = 20;
/*
	function iDraw() is called again and again by the system.

	*/

void iDraw()
{
	// place your drawing codes here

	if (running)
	{
		iClear();
		int count = 40;
		int piece_width = WIDTH / count;
		for (int i = 0, x = 0, y = 0; i < count; i++)
		{
			iSetColor(0, 50, 0);
			iLine(0, y, WIDTH, y);
			iLine(x, 0, x, WIDTH);
			y += piece_width;
			x += piece_width;
		}

		simulate_motion(&bodies, &body_count);
	}
	// iSetColor(20, 200, 200);
	// iFilledCircle(x, y, r);
	// iFilledRectangle(10, 30, 20, 20);
	// iSetColor(20, 200, 0);
	draw_button(*custom_btn);
	draw_button(*default_btn);
	draw_button(*count_btn);
}

/*
	function iMouseMove() is called when the user presses and drags the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouseMove(int mx, int my)
{
	// printf("x = %d, y= %d\n", mx, my);
	// place your codes here
	index_left = find_body_from_mouse(bodies, body_count, mx, my);
	// printf("Clicked on %d, (mx, my)=%d,%d\n", index_left, mx, my);
	if (index_left != -1)
	{
		bodies[index_left]->selected = true;
	}
	if (index_left != -1 && bodies[index_left]->selected)
	{
		bodies[index_left]->x = mx / SPACE_SCALE;
		bodies[index_left]->y = my / SPACE_SCALE;
		bodies[index_left]->selected = false;
		printf("Moved %d, to %d, %d\n", index_left, mx, my);
	}
}

/*
	function iMouse() is called when the user presses/releases the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouse(int button, int state, int mx, int my)
{

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		// place your codes here
		//	printf("x = %d, y= %d\n",mx,my);
		// x += 10;
		// y += 10;
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		// // place your codes here
		// x -= 10;
		// y -= 10;
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*default_btn, mx, my))
	{
		default_btn->selected = true;
		Body *body = create_body();
		body->x = (rand() % WIDTH) / SPACE_SCALE;
		body->y = (rand() % WIDTH) / SPACE_SCALE;
		body->radius = RADIUS_SCALE;
		body->velocity.y = rand() % 1000;
		body->velocity.x = rand() % 1000;
		for (int i = 0; i < 3; i++)
		{
			body->color[i] = rand() % 255;
		}

		append_body(body, &bodies, &body_count);
		// running = false;
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*custom_btn, mx, my))
	{
		custom_btn->selected = true;
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*count_btn, mx, my))
	{
		count_btn->selected = true;
	}

}

/*
	function iKeyboard() is called whenever the user hits a key in keyboard.
	key- holds the ASCII value of the key pressed.
	*/

// custom made planet is being made
void iKeyboard(unsigned char key)
{
	if (key == 'Q')
	{
		delete_all_bodies(&bodies, &body_count);
		delete_button(custom_btn);
		exit(0);
	}
	else if (key == 'P')
	{
		running = false;
	}
	else if (key == 'R')
	{
		running = true;
	}
	if (isdigit(key) == false && key != 13 && key != 8 && key != ',')
	{
		printf("Please enter digits, comma or newline.\n");
		return;
	}
	handle_custom_button(&custom_btn, key, &bodies, &body_count, &running);
	handle_count_button(&count_btn, key, &bodies, &body_count);
}

/*
	function iSpecialKeyboard() is called whenver user hits special keys like-
	function keys, home, end, pg up, pg down, arraows etc. you have to use
	appropriate constants to detect them. A list is:
	GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
	GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,
	GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP,
	GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
	*/
void iSpecialKeyboard(unsigned char key)
{

	if (key == GLUT_KEY_END)
	{
		exit(0);
	}
	// place your codes for other keys here
}

int main()
{
	// place your own initialization codes here.
	// iSetTimer(1000, iDraw);
	is_symmetric = true;
	running = true;

	default_btn->position.y -= default_btn->dimensions.y;
	strcpy(default_btn->str, "Create Planet");

	count_btn->position.y -= 2 * count_btn->dimensions.y;
	strcpy(count_btn->str, "Create Symmetric System");
	iInitialize(WIDTH, HEIGHT, "Universe Sandbox");
	return 0;
}
