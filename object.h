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
#define AU 1.496e11             // 1 Astronomical Unit = 1.496e11 meter
#define G 6.673e-11             // Universal Gravitational Constant
#define MAX_DISTANCE 70 * AU    // UNIT meter
#define MAX_VELOCITY 5e5        // UNIT: meter/second
#define SPACE_SCALE 7.63942e-11 // 800 Pixels = 70 AU
#define RADIUS_SCALE 300
#define COLLISION_FACTOR RADIUS_SCALE / 3.5
#define MASS_SCALE 1.6667e-25
#define G_SCALED 2.33665e-6 // after scaling the big G constant is G * SPACE_SCALE^2 / MASS_SCALE

typedef double pos_type;
typedef double lld;

lld TIMESTEP = 86400; // 1 hour

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
    body->x = (MAX_DISTANCE / 2);
    printf("%lf %lf\n", body->x, (MAX_DISTANCE / 2));
    body->y = (MAX_DISTANCE / 2);
    body->radius = 6.96e8;
    body->color[0] = 0;
    body->color[1] = 0;
    body->color[2] = 255;
    body->mass = 2e30;
    body->velocity = {0, 1e4};
    body->acceleration = {0, 0};
    return body;
}

pos_type pair_magnitude(Pair p)
{
    return sqrt(pow(p.x, 2) + pow(p.y, 2));
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

bool is_same_body(Body first, Body second)
{
    return (
        first.x == second.x && first.y == second.y && first.mass == second.mass && first.radius == second.radius && first.velocity.x == second.velocity.x && first.velocity.y == second.velocity.y && first.acceleration.x == second.acceleration.x && first.acceleration.y == first.acceleration.y);
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

void append_body(Body *new_body, Body ***bodies_ptr, int *body_count_ptr)
{
    int body_count = *body_count_ptr;
    *body_count_ptr += 1;

    if (*bodies_ptr == NULL)
        *bodies_ptr = create_default(*body_count_ptr);
    Body **bodies = *bodies_ptr;

    Body **tmp = (Body **)realloc(bodies, (*body_count_ptr) * sizeof(*bodies));

    if (tmp == NULL)
    {
        return;
    }
    *bodies_ptr = tmp; // Update the original pointer in the calling function
    (*bodies_ptr)[body_count] = new_body;
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

bool check_collision(Body body1, Body body2)
{
    // Calculate the distance between the centers of the two bodies
    pos_type d_square = pow((body2.x - body1.x), 2) + pow((body2.y - body1.y), 2);
    pos_type r_square = pow(COLLISION_FACTOR * (body1.radius + body2.radius), 2);
    // printf("d_square=%lf, r_square=%lf\n", d_square / (AU * AU), r_square / (AU * AU));
    // Check if the distance is less than the sum of the radii of the two bodies
    if (d_square <= r_square)
    {
        return true; // Collision detected
    }
    else
    {
        return false; // No collision
    }
}

void calculate_accelaration(Body *self, Body other)
{
    pos_type d_x = (other.x - self->x) * SPACE_SCALE;
    pos_type d_y = (other.y - self->y) * SPACE_SCALE;
    pos_type d_square = pow(d_x, 2) + pow(d_y, 2);
    double theta = atan2(d_y, d_x);

    pos_type mass = MASS_SCALE * other.mass;
    pos_type accelaration = G_SCALED * (mass / d_square);

    self->acceleration = {accelaration * cos(theta), accelaration * sin(theta)};
}

void handle_collision(Body *body1, Body *body2, Body ***bodies_ptr, int *body_count_ptr)
{
    Body *merged_body = create_body();
    // Calculate the total mass of the two bodies
    pos_type total_mass = body1->mass + body2->mass;
    merged_body->x = (body1->x + body2->x) / 2;
    merged_body->y = (body1->y + body2->y) / 2;
    merged_body->radius = sqrt(pow(body1->radius, 2) + pow(body2->radius, 2));
    merged_body->mass = total_mass;
    for (int i = 0; i < 3; i++)
    {
        merged_body->color[i] = (body1->color[i] + body2->color[i]) / 2;
    }

    // Calculate the new velocity of the combined body after collision
    merged_body->velocity.x = (body1->mass / total_mass) * body1->velocity.x + (body2->mass / total_mass) * body2->velocity.x;
    merged_body->velocity.y = (body1->mass / total_mass) * body1->velocity.y + (body2->mass / total_mass) * body2->velocity.y;

    delete_body(body1, bodies_ptr, body_count_ptr);
    delete_body(body2, bodies_ptr, body_count_ptr);
    append_body(merged_body, bodies_ptr, body_count_ptr);
}

// boundary collision just makes thing jarring. it's not pleasant or soothing
void handle_out_of_bounds(Body *body, Body **bodies, int *body_count_ptr)
{
    pos_type max_bound = 1.414 * MAX_DISTANCE;
    if (body->y > max_bound || body->y < max_bound / 100 || body->x > max_bound || body->x < max_bound / 100)
        delete_body(body, &bodies, body_count_ptr);
}

void update_position(int self_index, Body ***bodies_ptr, int *body_count_ptr)
{
    Body **bodies = *bodies_ptr;
    for (int i = 0; i < *body_count_ptr; i++)
    {
        // pos_type distance = sqrt(pow((bodies[i]->x - bodies[self_index]->x) * SPACE_SCALE, 2) + pow((bodies[i]->y - bodies[self_index]->y) * SPACE_SCALE, 2)) / SPACE_SCALE;
        if (i == self_index)
        {
        }
        else if (check_collision(*bodies[self_index], *bodies[i]))
        {
            handle_collision(bodies[self_index], bodies[i], bodies_ptr, body_count_ptr);
            bodies = *bodies_ptr;
            i--;
            continue;
        }

        if (i != self_index)
        {
            calculate_accelaration(bodies[self_index], *bodies[i]);
        }

        bodies[self_index]->velocity.x += bodies[self_index]->acceleration.x * TIMESTEP;
        bodies[self_index]->velocity.y += bodies[self_index]->acceleration.y * TIMESTEP;

        bodies[self_index]->x += bodies[self_index]->velocity.x * TIMESTEP;
        bodies[self_index]->y += bodies[self_index]->velocity.y * TIMESTEP;
    }
}

void draw_body(Body *body)
{
    int x = body->x * SPACE_SCALE;
    int y = body->y * SPACE_SCALE;
    int radius = body->radius * SPACE_SCALE * RADIUS_SCALE;
    // printf("x=%d, y=%d, r=%d, (%d, %d, %d) \n", x, y, radius, body->color[0], body->color[1], body->color[2]);

    iSetColor(body->color[0], body->color[1], body->color[2]);
    iFilledCircle(x, y, radius);

    // isn't working as expecting
    // no orbit is being drawn
    iPoint(x, y, 3);
}

void simulate_motion(Body ***bodies_ptr, int *body_count_ptr)
{
    Body **bodies = *bodies_ptr;
    for (int i = 0; i < *body_count_ptr; i++)
    {
        update_position(i, bodies_ptr, body_count_ptr);
        bodies = *bodies_ptr;
        draw_body(bodies[i]);
        printf("Body %d: x=%lfAU, y=%lfAU, v_x=%lfm/s, v_y=%lfm/s, a_x=%lf, a_y=%lf\n", i, bodies[i]->x / AU, bodies[i]->y / AU, bodies[i]->velocity.x, bodies[i]->velocity.y, bodies[i]->acceleration.x, bodies[i]->acceleration.y);
    }
}
