#include "iGraphics.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/*
in draw_body() function orbit drawing part, i.e. iLine() isn't working as desired
*/
#define WIDTH 800
#define HEIGHT 800
#define AU 1.496e11                 // 1 Astronomical Unit = 1.496e11 meter
#define G 6.673e-11                 // Universal Gravitational Constant
#define SPACE_SCALE (80.0 / 7) / AU // 800 Pixels = 70 AU
#define MASS_SCALE 1 / 6e24
#define G_SCALED 2.3232e-6    // after scaling the big G constant is G * SPACE_SCALE^2 / MASS_SCALE
#define MAX_DISTANCE 70       // UNIT AU
#define MAX_VELOCITY 3e8      // UNIT: m/s
#define MAX_ACCELARATION 1000 // Unit: m/s^2. max_accelaration it can handle is around AU / (10 * TIMESTEP^2)

typedef long double pos_type;
typedef long double lld;

lld TIMESTEP = 3600; // 1 hour

typedef struct Pair
{
    pos_type x;
    pos_type y;
} Pair;

typedef struct Body
{
    pos_type x;
    pos_type y;
    pos_type radius;
    int color[3];
    pos_type mass;
    Pair velocity;
    Pair acceleration;
} Body;

Body *create_body(void)
{
    Body *body = (Body *)malloc(sizeof(Body));
    body->x = (MAX_DISTANCE / 2) * AU;
    body->y = (MAX_DISTANCE / 2) * AU;
    body->radius = 6.96e8;
    body->color[0] = 0;
    body->color[1] = 0;
    body->color[2] = 255;
    body->mass = 2e30;
    body->velocity = {0, 1e4};
    body->acceleration = {0, 0};
    // body->velocity.x = 0;
    // body->velocity.y = 1e4;
    // body->acceleration.x = 0;
    // body->acceleration.y = 0;
    return body;
}

Body **create_default(int body_count)
{
    Body **bodies = (Body **)malloc(body_count * sizeof(*bodies));
    for (int i = 0; i < body_count; i++)
    {
        bodies[i] = create_body();
    }

    return bodies;
}

bool issame_body(Body first, Body second)
{
    return (first.x == second.x && first.y == second.y);
}

int find_body_index(Body body, Body **bodies, int body_count)
{
    for (int i = 0; i < body_count; i++)
    {
        if (issame_body(body, *bodies[i]))
            return i;
    }
    return -1;
}

/* swaps pointers between 2 bodies
 body_1 --> object1
 body_2 -->object2
 after swap
 body_1 --> object2
 body_2 --> object1
 */
// not working
void swap_body(Body **body_1, Body **body_2)
{
    Body *tmp = *body_1;
    *body_1 = *body_2;
    *body_2 = tmp;
}
// after calling this function decrease body_count by 1
void delete_body(Body *body, Body ***bodies_ptr, int *body_count_ptr)
{
    /*
    Swap last and deletion target body
    Then free target
    */
    int body_index = find_body_index(*body, *bodies_ptr, *body_count_ptr);
    free((*bodies_ptr)[body_index]);
    (*body_count_ptr)--;
    // now left shift everything by 1 space to make it (array) contiguous again
    for (int i = body_index; i < *body_count_ptr; i++)
    {
        (*bodies_ptr)[i] = (*bodies_ptr)[i + 1];
    }
    *bodies_ptr = (Body **)realloc(*bodies_ptr, (*body_count_ptr) * sizeof(**bodies_ptr));
}

// after calling this function reduce body_count to 0
void delete_bodies(Body **bodies, int body_count)
{
    for (int i = 0; i < body_count; i++)
        free(bodies[i]);
    free(bodies);
}

// after calling this function increase body_count by 1
void append_body(Body **bodies, int body_count)
{
    bodies = (Body **)realloc(bodies, (body_count + 1) * sizeof(*bodies));
    bodies[body_count] = create_body();
}

void calculate_accelaration(Body *self, Body other)
{
    pos_type d_x = (other.x - self->x) * SPACE_SCALE;
    pos_type d_y = (other.y - self->y) * SPACE_SCALE;
    pos_type d_square = pow(d_x, 2) + pow(d_y, 2);
    double theta = atan2(d_y, d_x);

    double mass = MASS_SCALE * other.mass;
    double accelaration = G_SCALED * (mass / d_square);

    if (accelaration >= MAX_ACCELARATION)
    {
        return;
    }
    // printf("%lf\n", accelaration);
    double a_x = accelaration * cos(theta);
    double a_y = accelaration * sin(theta);

    self->acceleration.x = a_x;
    self->acceleration.y = a_y;
}

bool check_collision(Body *body1, Body *body2)
{
    // Calculate the distance between the centers of the two bodies
    pos_type distance = sqrt(pow((body2->x - body1->x), 2) + pow((body2->y - body1->y), 2));

    // Check if the distance is less than the sum of the radii of the two bodies
    if (distance <= (body1->radius + body2->radius))
    {
        return true; // Collision detected
    }
    else
    {
        return false; // No collision
    }
}

void handle_collision(Body *body1, Body *body2)
{
    // Calculate the total mass of the two bodies
    pos_type total_mass = body1->mass + body2->mass;

    // Calculate the new velocity of the combined body after collision
    Pair new_velocity;
    new_velocity.x = (body1->mass * body1->velocity.x + body2->mass * body2->velocity.x) / total_mass;
    new_velocity.y = (body1->mass * body1->velocity.y + body2->mass * body2->velocity.y) / total_mass;

    // Update the velocity of both bodies
    body1->velocity = new_velocity;
    body2->velocity = new_velocity;
}

void handle_boundary_collision(Body *body)
{
    if (body->y >= MAX_DISTANCE * AU || body->y <= 0)
        body->velocity.y *= -1;
    if (body->x >= MAX_DISTANCE * AU || body->x <= 0)
        body->velocity.x *= -1;
}

void update_position(int self_index, Body **bodies, int body_count)
{
    for (int i = 0; i < body_count; i++)
    {
        if (i == self_index)
        {
            continue;
        }
        else if (check_collision(bodies[self_index], bodies[i]))
        {
            handle_collision(bodies[self_index], bodies[i]);
        }

        calculate_accelaration(bodies[self_index], *bodies[i]);

        pos_type v_x = bodies[self_index]->acceleration.x * TIMESTEP;
        pos_type v_y = bodies[self_index]->acceleration.y * TIMESTEP;
        if (sqrt(pow(v_x, 2) + pow(v_y, 2)) >= MAX_VELOCITY)
        {
            bodies[self_index]->velocity.x = 0;
            bodies[self_index]->velocity.y = 3e4;
            bodies[self_index]->x += bodies[self_index]->velocity.x * TIMESTEP;
            bodies[self_index]->y += bodies[self_index]->velocity.y * TIMESTEP;
            continue;
        }

        bodies[self_index]->velocity.x += bodies[self_index]->acceleration.x * TIMESTEP;
        bodies[self_index]->velocity.y += bodies[self_index]->acceleration.y * TIMESTEP;

        bodies[self_index]->x += bodies[self_index]->velocity.x * TIMESTEP;
        bodies[self_index]->y += bodies[self_index]->velocity.y * TIMESTEP;
        handle_boundary_collision(bodies[self_index]);
    }
}

void draw_body(Body *body, pos_type new_x, pos_type new_y, pos_type prev_x, pos_type prev_y)
{
    pos_type x = body->x * SPACE_SCALE;
    pos_type y = body->y * SPACE_SCALE;

    iSetColor(body->color[0], body->color[1], body->color[2]);
    iFilledCircle(x, y, body->radius * SPACE_SCALE);

    // isn't working as expecting
    // no orbit is being drawn
    iLine(new_x, new_y, prev_x, prev_y);
}

void simulate_motion(Body **bodies, int *body_count_ptr)
{
    for (int i = 0; i < *body_count_ptr; i++)
    {
        pos_type prev_x = bodies[i]->x, prev_y = bodies[i]->y;
        update_position(i, bodies, *body_count_ptr);
        pos_type new_x = bodies[i]->x, new_y = bodies[i]->y;
        draw_body(bodies[i], new_x, new_y, prev_x, prev_y);
        printf("Body %d: x=%LfAU, y=%LfAU, v_x=%Lfm/s, v_y=%Lfm/s, a_x=%Lf, a_y=%Lf\n", i, bodies[i]->x / AU, bodies[i]->y / AU, bodies[i]->velocity.x, bodies[i]->velocity.y, bodies[i]->acceleration.x, bodies[i]->acceleration.y);
    }
}
