// #include "iGraphics.h"
#include "object.h"

int body_count = 11;
int index_left = -1;
int str_index = 0;

// Body **bodies = create_solar_system(&body_count);
Body **bodies = create_symmetric_system(body_count);
Button *custom_btn = create_button();
Button *default_button = create_button();
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
	draw_button(*default_button);
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

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*default_button, mx, my))
	{
		default_button->selected = true;
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
		printf("Button selection successful.\n");
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && check_button_clicked(*custom_btn, mx, my))
	{
		custom_btn->selected = true;
		// running = false;
		printf("Button selection successful.\n");
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

	// default creation button
	if (isdigit(key) || key == 13 || key == 8 || key == ',')
	{
	}
	else
	{
		// iText(custom_btn->position.x, custom_btn->position.y - 3 * custom_btn->dimensions.y, "Please enter digits, comma or newline.\n");
		printf("Please enter digits, comma or newline.\n");
		return;
	}

	// this part handles string tokenization and custom planet creation
	// 13 is ascii value for \n
	if (key == 13 && custom_btn->selected)
	{
		char *tmp = (char *)malloc(sizeof(custom_btn->str));
		strcpy(tmp, custom_btn->str);
		char *token = strtok(tmp, ",");

		// there are 12 properties of a Body but accelaration and selection is always zero by default. just added random color
		int property_count = 6;
		double value_holder[property_count] = {0};
		int i = 0;
		while (token != NULL)
		{
			// printf("%s ", token);
			sscanf(token, "%lf", &value_holder[i]);
			i++;
			token = strtok(NULL, ",");
		}
		free(tmp);

		for (int i = 0; i < property_count; i++)
		{
			printf("%lf ", value_holder[i]);
		}
		Body *body = create_body();
		body->x = value_holder[0] / SPACE_SCALE;
		// printf("%lf\n", body->x);
		body->y = value_holder[1] / SPACE_SCALE;
		// printf("%lf\n", body->y);
		body->mass = value_holder[2] / MASS_SCALE;
		body->radius = value_holder[3] * RADIUS_SCALE;
		body->velocity.x = value_holder[4] * 1e3;
		body->velocity.y = value_holder[5] * 1e3;

		// printf("Body %d: radius=%lf, x=%lfAU, y=%lfAU, v_x=%lfm/s, v_y=%lfm/s, a_x=%lf, a_y=%lf\n", i, bodies[i]->radius, bodies[i]->x / AU, bodies[i]->y / AU, bodies[i]->velocity.x, bodies[i]->velocity.y, bodies[i]->acceleration.x, bodies[i]->acceleration.y);

		append_body(body, &bodies, &body_count);
		printf("\nSuccess\n");

		free(custom_btn->str);
		custom_btn->str = (char *)calloc(strlen("Create Custom Planet") + 1, sizeof(char));
		strcpy(custom_btn->str, "Create Custom Planet");
		custom_btn->selected = false;
		running = true;
	}

	if (custom_btn->selected)
	{
		if (strcmp(custom_btn->str, "Create Custom Planet") == 0)
			strcpy(custom_btn->str, "");
		custom_btn->str = char_cat(custom_btn->str, key);
		// printf("%c\n", key);
		// running = false;
	}

	int length = strlen(custom_btn->str);

	// 8 is ascii code for \b
	if (key == 8 && length > 0 && custom_btn->selected)
	{
		snprintf(custom_btn->str, length - 1, "%s", custom_btn->str);
	}
	// place your codes for other keys here
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

	default_button->position.y -= default_button->dimensions.y;
	strcpy(default_button->str, "Create Planet");
	iInitialize(WIDTH, HEIGHT, "Universe Sandbox");
	return 0;
}
