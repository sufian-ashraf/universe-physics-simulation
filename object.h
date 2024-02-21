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
#define AU 1.496e11                // 1 Astronomical Unit = 1.496e11 meter
#define G 6.673e-11                 // Universal Gravitational Constant
#define SPACE_SCALE 7.63942e-11 // 800 Pixels = 70 AU
#define MASS_SCALE 1.6667e-25
#define G_SCALED ((G / MASS_SCALE) * SPACE_SCALE) / SPACE_SCALE   // after scaling the big G constant is G * SPACE_SCALE^2 / MASS_SCALE
#define MAX_DISTANCE 70      // UNIT AU
#define MAX_VELOCITY 3e8      // UNIT: m/s
#define MAX_ACCELARATION 1000 // Unit: m/s^2. max_accelaration it can handle is around AU / (10 * TIMESTEP^2)

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
    body->x = (MAX_DISTANCE / 2) * AU;
    printf("%lf %lf\n", body->x, (MAX_DISTANCE / 2) * AU);
    body->y = (MAX_DISTANCE / 2) * AU;
    body->radius = 6.96e8;
    body->color[0] = 0;
    body->color[1] = 0;
    body->color[2] = 255;
    body->mass = 2e30;
    body->velocity = {0, 1e4};
    body->acceleration = {0, 0};
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
    return (first.x == second.x && first.y == second.y && first.mass == second.mass);
}

Pair calculate_accelaration(Body *self, Body other)
{
    pos_type d_x = (other.x - self->x) * SPACE_SCALE;
    pos_type d_y = (other.y - self->y) * SPACE_SCALE;
    pos_type d_square = pow(d_x, 2) + pow(d_y, 2);
    double theta = atan2(d_y, d_x);

    pos_type mass = MASS_SCALE * other.mass;
    pos_type accelaration = G_SCALED * (mass / d_square);

    Pair a = { accelaration * cos(theta), accelaration * sin(theta)};
    return a;
}

void update_position(int self_index, Body **bodies, int body_count)
{
    for (int i = 0; i < body_count; i++)
    {
        if (i == self_index)
        {
            continue;
        }

        Pair acceleration = calculate_accelaration(bodies[self_index], *bodies[i]);

        bodies[self_index]->velocity.x += acceleration.x * TIMESTEP;
        bodies[self_index]->velocity.y += acceleration.y * TIMESTEP;

        bodies[self_index]->x += bodies[self_index]->velocity.x * TIMESTEP;
        bodies[self_index]->y += bodies[self_index]->velocity.y * TIMESTEP;
    }
}

void draw_body(Body *body)
{
    int x = body->x * SPACE_SCALE;
    int y = body->y * SPACE_SCALE;
    int radius = body->radius * SPACE_SCALE * 300;
    printf("x=%d, y=%d, r=%d, (%d, %d, %d) \n", x, y, radius, body->color[0], body->color[1], body->color[2]);
    // pos_type x = body->x * SPACE_SCALE;
    // pos_type y = body->y * SPACE_SCALE;

    iSetColor(body->color[0], body->color[1], body->color[2]);
    iFilledCircle(x, y, radius);

    // isn't working as expecting
    // no orbit is being drawn
    // iLine(new_x, new_y, prev_x, prev_y);
}

void simulate_motion(Body **bodies, int body_count)
{
    for (int i = 0; i < body_count; i++)
    {
        // update_position(i, bodies, body_count);
        draw_body(bodies[i]);
        // printf("Body %d: x=%lfm, y=%lfm, v_x=%lfm/s, v_y=%lfm/s, a_x=%lf, a_y=%lf\n", i, bodies[i]->x, bodies[i]->y, bodies[i]->velocity.x, bodies[i]->velocity.y, bodies[i]->acceleration.x, bodies[i]->acceleration.y);
    }
}
