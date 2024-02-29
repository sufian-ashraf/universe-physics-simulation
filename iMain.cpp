// #include "iGraphics.h"
#include "object.h"

int body_count = 0;
int index_left = -1;

FILE *file = fopen("planets.txt", "r");
Body **bodies = create_bodies(&body_count, file);
Button *custom_btn = create_button();
Button *default_btn = create_button();
Button *symmetric_btn = create_button();
Button *modification_btn = create_button();
Button *collection_btn = create_button();
int mod_body_index = -1;
/*
	function iDraw() is called again and again by the system.

	*/

void iDraw()
{
	// place your drawing codes here
	if (running)
	{
		if (time_skip == false)
		{
			iClear();
			draw_grid_lines();
		}
		simulate_motion(&bodies, &body_count, time_skip);
	}
	draw_button(*custom_btn);
	draw_button(*default_btn);
	draw_button(*symmetric_btn);
	draw_button(*collection_btn);
	if (modification_btn->selected)
	{
		draw_button(*modification_btn);
	}
}

/*
	function iMouseMove() is called when the user presses and drags the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouseMove(int mx, int my)
{

	index_left = find_body_from_mouse(bodies, body_count, mx, my);
	if (index_left != -1)
	{
		bodies[index_left]->selected = true;
	}
	if (index_left != -1 && bodies[index_left]->selected)
	{
		bodies[index_left]->position.x = mx / SPACE_SCALE;
		bodies[index_left]->position.y = my / SPACE_SCALE;
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
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*default_btn, mx, my))
	{
		default_btn->selected = true;
		int vel_range = 1000 + rand() % 5000;
		Body *body = create_body((rand() % WIDTH), (rand() % HEIGHT), 1, 16, rand() % vel_range, rand() % vel_range);
		append_body(body, &bodies, &body_count);
		default_btn->selected = false;
		// running = false;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*collection_btn, mx, my))
	{
		int cluster_count = 10;
		collection_btn->selected = true;
		int vel_range = 1000 + rand() % 5000;
		for (int i = 0; i < cluster_count; i++)
		{
			Body *body = create_body((rand() % WIDTH), (rand() % HEIGHT), 1, 8, (rand() % vel_range) * pow(-1, rand() % 2), (rand() % vel_range) * pow(-1, rand() % 2));
			append_body(body, &bodies, &body_count);
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*custom_btn, mx, my))
	{
		strcpy(custom_btn->str, "");
		custom_btn->selected = true;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*symmetric_btn, mx, my))
	{
		strcpy(symmetric_btn->str, "");
		symmetric_btn->selected = true;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		modification_btn->selected = true;
		mod_body_index = find_body_from_mouse(bodies, body_count, mx, my);
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
		delete_button(modification_btn);
		delete_button(default_btn);
		delete_button(symmetric_btn);
		fclose(file);
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
	else if (key == 'T')
	{
		time_skip = !time_skip;
	}
	else if (key == 'N')
	{
		newtons_formula_on = !newtons_formula_on;
	}
	else if (key == 'I')
	{
		delete_all_bodies(&bodies, &body_count);
		bodies = create_bodies(&body_count, file);
	}
	else if (isdigit(key) == false && key != 13 && key != 8 && key != ',')
	{
		printf("Please enter digits, comma, newline etc.\n");
		return;
	}
	handle_custom_button(&custom_btn, key, &bodies, &body_count, &running);
	handle_symmetric_button(&symmetric_btn, key, &bodies, &body_count);
	handle_modification_button(&modification_btn, key, &bodies, &body_count, mod_body_index);
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
	collision_on = false;
	running = true;
	time_skip = false;
	default_btn->position.y -= default_btn->dimensions.y;
	strcpy(default_btn->str, "Create Planet");

	symmetric_btn->position.y -= 2 * symmetric_btn->dimensions.y;
	strcpy(symmetric_btn->str, "Create Symmetric System");

	collection_btn->position.y -= 3 * collection_btn->dimensions.y;
	strcpy(collection_btn->str, "Create Cluster");

	modification_btn->dimensions.x = 250;
	modification_btn->position.x = WIDTH / 2 - modification_btn->dimensions.x / 2;
	modification_btn->position.y = HEIGHT - modification_btn->dimensions.y;
	modification_btn->str = (char *)realloc(modification_btn->str, (strlen("Mass, radius, vel x, vel y") + 1) * sizeof(char));
	strcpy(modification_btn->str, "Mass, radius, vel x, vel y");

	iInitialize(WIDTH, HEIGHT, "Universe Sandbox");
	return 0;
}
