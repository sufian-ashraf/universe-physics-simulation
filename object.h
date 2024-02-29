#include "iGraphics.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
in draw_body() function orbit drawing part, i.e. iLine() isn't working as desired
*/
#define WIDTH 800
#define HEIGHT 800
#define AU 1.496e11                                            // 1 Astronomical Unit = 1.496e11 meter
#define G 6.673e-11                                            // Universal Gravitational Constant
#define MAX_DISTANCE (70 * AU)                                 // UNIT meter
#define MAX_VELOCITY 5e5                                       // UNIT: meter/second
#define SPACE_SCALE ((WIDTH / 100.0) / (MAX_DISTANCE / 100.0)) // 800 Pixels = 70 AU
#define DRAW_SCALE 1
#define COLLISION_FACTOR 1
#define MASS_SCALE 1.6667e-25
#define RADIUS_SCALE 28.845e-3 // to set sun's radius equal to double of earth's pixel radius w.r.t to mass
#define MAX_SPEED 7e3
#define AVG_DENSITY 3600 // Unit kg/m^3. this value is avg of sun and earths density
#define G_SCALED 2.3242e-6 // after scaling the big G constant is G * SPACE_SCALE^2 / MASS_SCALE
#define PI 3.141592

typedef double pos_type;

pos_type TIMESTEP = 86400; // 1 day
bool collision_on = false;
bool running = true;
bool time_skip = false;
bool newtons_formula_on = true;

typedef struct Pair
{
    pos_type x;
    pos_type y;
} Pair;

typedef struct Body
{
    Pair position;
    pos_type radius;
    bool selected;
    pos_type mass;
    Pair velocity;
    Pair acceleration;
    int color[3];
} Body;

// these x,y,mass,radiusvalues only takes in scaled values
// but they are converted and stored in real,actual  values
Body *create_body(double pos_x, double pos_y, double mass, double radius, double vel_x, double vel_y)
{
    Body *body = (Body *)malloc(sizeof(Body));

    if (body == NULL)
    {
        // Handle memory allocation failure
        printf("Memory allocation failed\n");
        return NULL;
    }
    body->position.x = pos_x / SPACE_SCALE;
    body->position.y = pos_y / SPACE_SCALE;
    body->mass = mass / MASS_SCALE;
    body->radius = (radius + log(mass)) / SPACE_SCALE;
    body->velocity.x = vel_x;
    body->velocity.y = vel_y;
    body->acceleration.x = 0;
    body->acceleration.y = 0;
    body->selected = false;
    for (int i = 0; i < 3; i++)
        body->color[i] = rand() % 256;

    return body;
}

int append_body(Body *new_body, Body ***bodies_ptr, int *body_count_ptr)
{
    int body_count = *body_count_ptr;
    *body_count_ptr += 1;

    if (*bodies_ptr == NULL)
    {
        *bodies_ptr = (Body **)malloc(*body_count_ptr * sizeof(Body *));
    }
    Body **bodies = *bodies_ptr;
    Body **tmp = bodies;
    int k = 0;
    // printf("Append reallocation count:\n");
    do
    {
        k++;
        tmp = (Body **)realloc(bodies, (*body_count_ptr) * sizeof(Body *));

    } while (tmp == NULL);

    if (tmp == NULL)
    {
        return -1;
    }
    bodies = tmp;
    *bodies_ptr = bodies; // Update the original pointer in the calling function
    (*bodies_ptr)[body_count] = new_body;
    bodies = (*bodies_ptr);

    return 0;
}

pos_type pair_magnitude(Pair p)
{
    return sqrt(pow(p.x, 2) + pow(p.y, 2));
}

Body **create_bodies(int *body_count_ptr, FILE *file)
{
    if (file == NULL)
    {
        Body **bodies = (Body **)malloc(*body_count_ptr * sizeof(*bodies));
        for (int i = 0; i < *body_count_ptr; i++)
        {
            Body *body = create_body((rand() % WIDTH), (rand() % HEIGHT), 1, 16, rand() % 1000, rand() % 1000);
            append_body(body, &bodies, body_count_ptr);
        }
        return bodies;
    }

    fseek(file, 0, SEEK_SET);
    int count = 0, buffer_size = 512;
    char *buffer = (char *)calloc(buffer_size, sizeof(char));
    Body **bodies = (Body **)malloc(*body_count_ptr * sizeof(*bodies));

    // this part skips first line of file
    int c;
    while ((c = fgetc(file)) != EOF && c != '\n')
    {
    }

    while (fgets(buffer, buffer_size, file) != NULL)
    {
        // properties of body, not counting colors, accelaration and selection
        double properties[6];

        // parse the csv and get property values
        char *token = strtok(buffer, ",");
        for (int i = 0; token != NULL; i++)
        {
            sscanf(token, "%lf", &properties[i]);
            token = strtok(NULL, ",");
        }

        Body *body = create_body(properties[0], properties[1], properties[2], properties[3], properties[4], properties[5]);
        append_body(body, &bodies, body_count_ptr);
        count++;
    }

    free(buffer);

    *body_count_ptr = count;
    return bodies;
}

Body **create_symmetric_system(int body_count)
{
    Body **bodies = (Body **)malloc(body_count * sizeof(*bodies));
    pos_type vertex_count = body_count - 1;
    srand(time(NULL));
    pos_type center_x = MAX_DISTANCE / 2;
    pos_type center_y = MAX_DISTANCE / 2;
    pos_type common_mass = 6e24;
    pos_type distance = 10 * AU; // from centroid center

    pos_type vel_factor = 0;
    for (int del_i = 1; del_i < vertex_count; del_i++)
    {
        pos_type c_val = cos((PI * del_i) / vertex_count);
        bool flag = (2 * del_i == vertex_count);
        if (flag == true)
        {
            vel_factor += 1;
            continue;
        }
        vel_factor += pow(1 / c_val, 2);
    }

    pos_type common_vel = (1e4 / vertex_count) + sqrt((G * common_mass * vel_factor) / (4 * distance));
    pos_type common_radius = 12; // scaled value
    for (int i = 0; i < vertex_count; i++)
    {
        pos_type normal_x = cos((2 * PI * i) / vertex_count);
        pos_type normal_y = sin((2 * PI * i) / vertex_count);
        bodies[i] = create_body((center_x + normal_x * distance) * SPACE_SCALE, (center_y + normal_y * distance) * SPACE_SCALE, common_mass * MASS_SCALE, common_radius, (-normal_y * common_vel), (normal_x * common_vel));
    }

    bodies[body_count - 1] = create_body(center_x * SPACE_SCALE, center_y * SPACE_SCALE, 2e30 * MASS_SCALE, common_radius, 0, 0); // Sun

    return bodies;
}

bool is_same_body(Body first, Body second)
{
    return (
        first.position.x == second.position.x && first.position.y == second.position.y && first.mass == second.mass && first.radius == second.radius && first.velocity.x == second.velocity.x && first.velocity.y == second.velocity.y && first.acceleration.x == second.acceleration.x && first.acceleration.y == first.acceleration.y);
}

int find_body_index(Body body, Body **bodies, int body_count)
{
    for (int i = 0; i < body_count; i++)
    {
        if (is_same_body(body, *bodies[i]))
            return i;
    }
    return -1;
}

void delete_body(Body *body, Body ***bodies_ptr, int *body_count_ptr)
{
    int body_count = *body_count_ptr;
    // Find the index of the body to be deleted
    int body_index = find_body_index(*body, *bodies_ptr, body_count);

    // Body not found in the array
    if (body_index == -1)
    {
        return;
    }

    // Free the body itself
    free((*bodies_ptr)[body_index]);

    // Shift elements to the left to fill the gap
    for (int i = body_index; i < body_count - 1; i++)
    {
        (*bodies_ptr)[i] = (*bodies_ptr)[i + 1];
    }

    // Nullify the last element to avoid dangling pointers
    (*bodies_ptr)[body_count - 1] = NULL;

    // Reduce memory block size
    Body **temp = (Body **)realloc(*bodies_ptr, (body_count - 1) * sizeof(*(*bodies_ptr)));
    *body_count_ptr = body_count - 1;
    if (temp == NULL)
    {
        // Handle reallocation failure
        *bodies_ptr = NULL;
        return;
    }

    // Update bodies pointer and body count
    *bodies_ptr = temp;
}

void delete_all_bodies(Body ***bodies_ptr, int *body_count_ptr)
{
    while (*body_count_ptr != 0)
    {
        delete_body((*bodies_ptr)[0], bodies_ptr, body_count_ptr);
    }
}


void calculate_accelaration(Body *self, Body other)
{
    pos_type d_x = (other.position.x - self->position.x) * SPACE_SCALE;
    pos_type d_y = (other.position.y - self->position.y) * SPACE_SCALE;
    pos_type d_square = pow(d_x, 2) + pow(d_y, 2);
    double theta = atan2(d_y, d_x);

    pos_type mass = MASS_SCALE * other.mass;
    pos_type accelaration = 0;
        accelaration = G_SCALED * (mass / d_square);

    if (newtons_formula_on == false)
    {
        pos_type strength = 0.01;
        accelaration += strength / log(AU + sqrt(d_square) / SPACE_SCALE);
    }
    self->acceleration = {accelaration * cos(theta), accelaration * sin(theta)};
}


void update_position(Body *body, Body ***bodies_ptr, int *body_count_ptr)
{
    Body **bodies = *bodies_ptr;
    int self_index = find_body_index(*body, *bodies_ptr, *body_count_ptr);
    for (int i = 0; i < *body_count_ptr; i++)
    {
        if (i != self_index)
        {
            calculate_accelaration(bodies[self_index], *bodies[i]);
        }

        pos_type u_x = bodies[self_index]->velocity.x;
        pos_type u_y = bodies[self_index]->velocity.y;
        bodies[self_index]->velocity.x += bodies[self_index]->acceleration.x * TIMESTEP;
        bodies[self_index]->velocity.y += bodies[self_index]->acceleration.y * TIMESTEP;

        bodies[self_index]->position.x += (u_x + bodies[self_index]->velocity.x) * (TIMESTEP / 2);
        bodies[self_index]->position.y += (u_y + bodies[self_index]->velocity.y) * (TIMESTEP / 2);

    }
}

void draw_body(Body *body)
{
    int x = body->position.x * SPACE_SCALE;
    int y = body->position.y * SPACE_SCALE;
    int radius = body->radius * SPACE_SCALE * DRAW_SCALE;

    double theta = atan2(body->velocity.y, body->velocity.x);
    int end_x = x + 2 * radius * cos(theta);
    int end_y = y + 2 * radius * sin(theta);

    iSetColor(body->color[0], body->color[1], body->color[2]);
    iFilledCircle(x, y, radius);

    iLine(x, y, end_x, end_y);
    iFilledCircle(end_x, end_y, 2, 3);
    // isn't working as expecting
    // no orbit is being drawn
}

void simulate_motion(Body ***bodies_ptr, int *body_count_ptr, bool time_skip)
{
    Body **bodies = *bodies_ptr;
    for (int i = 0; i < *body_count_ptr; i++)
    {
        update_position(bodies[i], bodies_ptr, body_count_ptr);
        bodies = *bodies_ptr;
        if (time_skip == false)
        {
            draw_body(bodies[i]);
        }
    }
}

int find_body_from_mouse(Body **bodies, int body_count, int mx, int my)
{
    for (int i = 0; i < body_count; i++)
    {
        int x = bodies[i]->position.x * SPACE_SCALE;
        int y = bodies[i]->position.y * SPACE_SCALE;
        pos_type width = bodies[i]->radius * SPACE_SCALE * DRAW_SCALE;
        if ((mx <= x + width && mx >= x - width) && (my >= y - width && my <= y + width))
            return i;
    }
    return -1;
}

void draw_grid_lines(void)
{
    int grid_count = 40;
    int piece_width = WIDTH / grid_count;
    for (int i = 0, x = 0, y = 0; i < grid_count; i++)
    {
        iSetColor(0, 40, 0);
        iLine(0, y, WIDTH, y);
        iLine(x, 0, x, WIDTH);
        y += piece_width;
        x += piece_width;
    }
}

/*
    Button object related code
*/
char *char_cat(char *s, char ch)
{
    char *tmp = (char *)malloc(sizeof(char));
    sprintf(tmp, "%c", ch);
    strcat(s, tmp);
    free(tmp);
    return s;
}

typedef struct Button
{
    Pair position;
    Pair dimensions;
    char *str;
    int bg_color[3];
    int txt_color[3];
    bool selected;
} Button;

Button *create_button(void)
{
    Button *button = (Button *)malloc(sizeof(Button));

    *button = {.position = {.x = WIDTH - 200, .y = HEIGHT - 50}, .dimensions = {.x = 200, .y = 50}, .str = NULL, .bg_color = {0, 50, 137}, .txt_color = {255, 127, 123}};
    button->str = (char *)calloc(strlen("Create Custom Planet") + 1, sizeof(char));
    strcpy(button->str, "Create Custom Planet");
    return button;
}

void delete_button(Button *button)
{
    free(button->str);
    free(button);
}

void draw_button(Button button)
{
    iSetColor(button.bg_color[0], button.bg_color[1], button.bg_color[2]);
    iFilledRectangle(button.position.x, button.position.y, button.dimensions.x, button.dimensions.y);

    iSetColor(255 - button.bg_color[0], 255 - button.bg_color[1], 255 - button.bg_color[2]);
    iRectangle(button.position.x, button.position.y, button.dimensions.x, button.dimensions.y);

    iSetColor(button.txt_color[0], button.txt_color[1], button.txt_color[2]);
    iText(button.position.x, button.position.y + (button.dimensions.y / 2), button.str);
}

bool check_button_clicked(Button button, int mx, int my)
{
    int button_x = button.position.x, button_y = button.position.y;
    if ((mx >= button_x && mx <= button_x + button.dimensions.x) && (my >= button_y && my <= button_y + button.dimensions.y))
    {
        return true;
    }
    return false;
}

void handle_custom_button(Button **btn_ptr, unsigned char key, Body ***bodies_ptr, int *body_count_ptr, bool *running_ptr)
{
    Button *custom_btn = *btn_ptr;
    if (custom_btn->selected == false)
    {
        return;
    }

    // this part handles string tokenization and custom planet creation
    // 13 is ascii value for \n
    int custom_str_len = strlen(custom_btn->str);
    if (key == 13)
    {
        int property_count = 6;
        double properties[property_count] = {0};
        char *tmp_str = (char *)malloc(strlen(custom_btn->str) + 1);
        strcpy(tmp_str, custom_btn->str);
        char *token = strtok(tmp_str, ",");

        // there are 12 properties of a Body but accelaration and selection is always zero by default. just added random color
        for (int i = 0; token != NULL; i++)
        {
            sscanf(token, "%lf", &properties[i]);
            token = strtok(NULL, ",");
        }
        free(tmp_str);

        Body *body = create_body(properties[0], properties[1], properties[2], properties[3], properties[4], properties[5]);
        if (body == NULL)
        {
            printf("Custom body creation failed.\n");
            return;
        }

        append_body(body, bodies_ptr, body_count_ptr);

        custom_btn->str = (char *)realloc(custom_btn->str, (strlen("Create Custom Planet") + 1) * sizeof(char));
        if (custom_btn->str == NULL)
        {
            printf("Custom button str reallocation failed.\n");
        }
        strcpy(custom_btn->str, "Create Custom Planet");

        custom_btn->selected = false;
        *running_ptr = true;
        *btn_ptr = custom_btn;

        return;
    }
    else if (key == 8 && custom_str_len > 0)
    {
        // 8 is ascii code for \b
        snprintf(custom_btn->str, custom_str_len, "%s", custom_btn->str);
    }
    else if (isdigit(key) || key == ',' || key == '-')
    {
        custom_btn->str = char_cat(custom_btn->str, key);
    }

    *btn_ptr = custom_btn;
}

void handle_symmetric_button(Button **btn_ptr, unsigned char key, Body ***bodies_ptr, int *body_count_ptr)
{

    Button *symmetric_btn = *btn_ptr;
    int len = strlen(symmetric_btn->str);

    if (symmetric_btn->selected == false)
    {
        return;
    }

    // ascii code for \n is 13
    if (key == 13)
    {
        delete_all_bodies(bodies_ptr, body_count_ptr);
        sscanf(symmetric_btn->str, "%d", body_count_ptr);
        *bodies_ptr = create_symmetric_system(*body_count_ptr);

        free(symmetric_btn->str);
        symmetric_btn->str = (char *)calloc(strlen("Create Symmetric System") + 1, sizeof(char));
        strcpy(symmetric_btn->str, "Create Symmetric System");
    }
    else if (key == 8 && len > 0 && symmetric_btn->selected)
    {
        // 8 is ascii code for \b
        snprintf(symmetric_btn->str, len, "%s", symmetric_btn->str);
    }
    else if (symmetric_btn->selected)
    {
        symmetric_btn->str = char_cat(symmetric_btn->str, key);
    }
}

void handle_modification_button(Button **btn_ptr, unsigned char key, Body ***bodies_ptr, int *body_count_ptr, int body_index)
{
    Button *modification_btn = *btn_ptr;
    if (modification_btn->selected == false)
    {
        return;
    }

    int modification_strlen = strlen(modification_btn->str);
    if (strcmp(modification_btn->str, "Mass, radius, vel x, vel y") == 0)
    {
        strcpy(modification_btn->str, "");
    }
    if (key == 13)
    {
        // modifiable properties mass,radius, vel_x, vel_y
        int modifiable_property_count = 4;
        double modifiable_properties[modifiable_property_count] = {0};

        char *tmp_str = (char *)malloc((strlen(modification_btn->str) + 1) * sizeof(char));
        strcpy(tmp_str, modification_btn->str);
        char *token = strtok(tmp_str, ",");

        for (int i = 0; i < modifiable_property_count && token != NULL; i++)
        {
            sscanf(token, "%lf", &modifiable_properties[i]);
            token = strtok(NULL, ",");
        }
        free(tmp_str);

        modification_btn->str = (char *)realloc(modification_btn->str, (strlen("Mass, radius, vel x, vel y") + 1) * sizeof(char));
        if (modification_btn->str == NULL)
        {
            printf("Modification button str reallocation error.\n");
        }
        strcpy(modification_btn->str, "Mass, radius, vel x, vel y");

        Body *body = create_body((*bodies_ptr)[body_index]->position.x * SPACE_SCALE, (*bodies_ptr)[body_index]->position.y * SPACE_SCALE, modifiable_properties[0], modifiable_properties[1], modifiable_properties[2], modifiable_properties[3]);
        delete_body((*bodies_ptr)[body_index], bodies_ptr, body_count_ptr);
        append_body(body, bodies_ptr, body_count_ptr);

        modification_btn->selected = false;
        *btn_ptr = modification_btn;
        return;
    }
    else if (key == 8 && modification_strlen > 0)
    {
        // 8 is ascii code for \b
        snprintf(modification_btn->str, modification_strlen, "%s", modification_btn->str);
    }
    else if (isdigit(key) || key == ',' || key == '-')
    {
        modification_btn->str = char_cat(modification_btn->str, key);
    }

    *btn_ptr = modification_btn;
}