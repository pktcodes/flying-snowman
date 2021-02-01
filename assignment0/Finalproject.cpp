//authors : Praveen

//this project is done in Xcode on Mac


#include <stdlib.h>
#include <math.h>
#include <cstdio>
#include <cstring>

#ifndef OSX
#ifndef LINUX
#define WINDOWS 1
#endif
#endif

#if OSX
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#endif

#if WINDOWS
#include "../shared/gltools.h"        // OpenGL toolkit
#endif

#if LINUX
#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>
#endif

template<typename T>
T
min_f(T a, T b)
{
    return a < b ? a : b;
}

template<typename T>
T
max_f(T a, T b)
{
    return a > b ? a : b;
}

enum game_state_t
{
    BEGINNING = 0,
    RUNNING = 1,
    HOLA = 2
};

const int MAX_OBSTACLES = 9;
const float OBSTACLE_PROBABILITY = 0.1;

struct game_context_t
{
    struct splash_t {
        int state;
        int current_symbol;
        int wait_steps;
        char msg[4096];
        int string_length;
    } intro_screen, end_screen;
    struct platform_t {
        int state;
        struct tree_t {
            float pos_x;
            float pos_y;
            float pos_z;
            bool active;
            bool broken;
            bool scored;
        } tree[MAX_OBSTACLES];
        struct player_t {
            float pos_y;
            float force;
            int lives;
            int score;
        } player;
    } platform;
};

game_context_t global_game_context;

const int TIMER_PERIOD = 30;
const float TREE_MOVE_SPEED = 5e-3;
const float SNOWMAN_UP_FORCE_STEP = 0.3;
const float SNOWMAN_DOWN_FORCE_STEP = 0.02;
const float SNOWMAN_MAX_FORCE = 0.5;
const float SNOWMAN_MIN_FORCE = -0.2;
const int SNOWMAN_MAX_LIVES = 3;

int global_game_step;

game_state_t global_game_state;
void timer_callback(int);
void display_intro();
void reshape_callback(int w, int h);
void render_platform();
void reinit_obstacles();
void populate_obstacles();
void add_obstacle();
void display_obstacles();
void display_obstacle(game_context_t::platform_t::tree_t* tree);
float get_ar();
void draw_snowman(float red_blend);
void raise_player_energy();
void update_player_energy();
void update_platform_state();
void circulartree(float color_multiplier);
void display_end_game();
void render_text(game_context_t::splash_t* ctx);
void reset_game();


float posX = 0.01, posY = -0.1, posZ = 0,


sx1 = 1.2, sy1 = -0.2,
sx2 = -0.0, sy2 = 0.9,
sx3 = 1.20, sy3 = 1.10;




GLfloat rotation = 90.0;
double x, y, angle;
#define PI 3.1415926535898


int theta = -32, phi = 22;

//teacups
void draw_snowman(float red_blend) {
    glPushMatrix();
    glColor3f(
        1.0 * (1 - red_blend) + red_blend * 1.0,
        1.0 * (1 - red_blend),
        1.0 * (1 - red_blend)
    );
    glTranslatef(sx1, sy1, 0.0);
    glutSolidSphere(0.7, 20, 20);
    glColor3f(
        0.9 * (1 - red_blend) + red_blend * 1.0,
        0.9 * (1 - red_blend),
        0.9 * (1 - red_blend)
    );
    glTranslatef(sx2, sy2, 0.0);
    glutSolidSphere(0.4, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.9, 0.9, 0.0);
    glTranslatef(1.2, 1.3, 0.0f);
    glRotatef(120.0, 1.4, 1.0, 0.0);
    glBegin(GL_POLYGON);    // Draw A Quad
    glVertex3f(-0.7f, 1.0, 0.0f);    // Top Left
    glVertex3f(0.7f, 1.0, 0.0f);    // Top Right
    glVertex3f(0.7f, -1.0f, 0.0f);    // Bottom Right
    glVertex3f(-0.7f, -1.0f, 0.0f);    // Bot
    glEnd();
    glPopMatrix();
}



void circulartree(float color_multiplier)
{
    glPushMatrix();
    glColor3f(0.4 * color_multiplier, 0.3 * color_multiplier, 0.3 * color_multiplier);
    glScalef(1, 4, 1);
    glTranslatef(0.0, 0.0, 0.0);
    glutSolidCube(0.25);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0 * color_multiplier, 1 * color_multiplier, 0 * color_multiplier);
    glTranslatef(0.0, 0.6, 0.0);
    glutSolidSphere(0.4, 6.0, 10);
    glPopMatrix();

}


void keys(unsigned char a, int x, int y) {
    switch (a) {

    case 'w':
        theta -= 20;
        break;

    case 's':
        theta += 20;
        break;

    case 'a':
        phi -= 20;
        break;

    case 'd':
        phi += 20;
        break;
    case ' ':
        if (global_game_state == game_state_t::RUNNING)
        {
            raise_player_energy();
        }
        break;
    case 13:
        if (global_game_state == game_state_t::BEGINNING)
        {
            global_game_state = game_state_t::RUNNING;
        }
        else if (global_game_state == game_state_t::HOLA)
        {
            global_game_state = game_state_t::BEGINNING;
            reset_game();
        }
        break;
    case 'q':
        exit(0);
        break;
    default:
        break;
    }

    glutPostRedisplay();

}

void rotate() {
    if (theta >= 180)
        theta = -180;
    else if (theta <= -180)
        theta = 180;
    phi = phi % 360;

    glutPostRedisplay();
}


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (global_game_state == game_state_t::BEGINNING)
    {
        display_intro();
    }
    else if (global_game_state == game_state_t::RUNNING)
    {
        render_platform();
    }
    else if (global_game_state == game_state_t::HOLA)
    {
        display_end_game();
    }
    else
    {
        fprintf(
            stderr,
            "display: unknown game state %d\n", (int)(global_game_state)
        );
    }

    glutSwapBuffers();
    return;
}

void reinit_obstacles()
{
    for (int i = 0; i < MAX_OBSTACLES; ++i)
    {
        global_game_context.platform.tree[i].active = false;
    }
}

void add_obstacle()
{
    bool found = false;

    for (int i = 0; i < MAX_OBSTACLES; ++i)
    {
        if (!global_game_context.platform.tree[i].active)
        {
            global_game_context.platform.tree[i].active = true;
            global_game_context.platform.tree[i].broken = false;
            global_game_context.platform.tree[i].scored = false;
            global_game_context.platform.tree[i].pos_x = 1.0;
            global_game_context.platform.tree[i].pos_y = 1.0 * (rand() % 100) / 100;
            global_game_context.platform.tree[i].pos_z = 0;
            found = true;
            break;
        }
    }

    if (!found)
    {
        fprintf(
            stderr,
            "add_obstacle: not found a free obstacle entry\n"
        );
    }
}

void populate_obstacles()
{
    int num_free_obstacles = 0;

    for (int i = 0; i < MAX_OBSTACLES; ++i)
    {
        if (global_game_context.platform.tree[i].active)
        {
            ++num_free_obstacles;
        }
    }

    if (num_free_obstacles < MAX_OBSTACLES)
    {
        if (1.0 * (rand() % 100) / 100 < OBSTACLE_PROBABILITY)
        {
            add_obstacle();
        }
    }
}

void display_obstacles()
{
    for (int i = 0; i < MAX_OBSTACLES; ++i)
    {
        if (global_game_context.platform.tree[i].active)
        {
            display_obstacle(&global_game_context.platform.tree[i]);

            global_game_context.platform.tree[i].pos_x =
                max_f(
                    global_game_context.platform.tree[i].pos_x - TREE_MOVE_SPEED,
                    0.0f
                );

            if (global_game_context.platform.tree[i].pos_x <= 1e-8)
            {
                global_game_context.platform.tree[i].active = false;
            }


        }
    }
}

void update_platform_state()
{
    for (int i = 0; i < MAX_OBSTACLES; ++i)
    {
        if (
            global_game_context.platform.tree[i].active &&
            !global_game_context.platform.tree[i].broken &&
            !global_game_context.platform.tree[i].scored
            )
        {
            float x1 = global_game_context.platform.tree[i].pos_x;
            float y1 = global_game_context.platform.tree[i].pos_y;

            float x2 = 0.5;
            float y2 = global_game_context.platform.player.pos_y;

            float dx = (x1 - x2);
            float dy = (y1 - y2);

            float dist = dx * dx + dy * dy;

#if 0
            printf(
                "%d:%5.2f:%5.2f:%5.2f:%5.2f:%5.2f:%5.2f:%5.2f\n",
                i, x1, y1, x2, y2, dx, dy, dist
            );
#endif

            if (dist < 0.01)
            {
                global_game_context.platform.player.lives =
                    max_f(
                        global_game_context.platform.player.lives - 1,
                        0
                    );
                global_game_context.platform.tree[i].broken = true;
            }
            else if (x1 < x2 - 0.03)
            {
                global_game_context.platform.tree[i].scored = true;
                global_game_context.platform.player.score += 1;
            }
        }
    }

    if (global_game_context.platform.player.lives == 0)
    {
        global_game_state = game_state_t::HOLA;
    }
}

void raise_player_energy()
{
    global_game_context.platform.player.force =
        min_f(
            global_game_context.platform.player.force + SNOWMAN_UP_FORCE_STEP,
            SNOWMAN_MAX_FORCE
        );
}

void update_player_energy()
{
    float t1 = global_game_context.platform.player.force;
    float dy = t1 * t1;
    if (t1 < 0)
    {
        dy *= -1;
    }

    global_game_context.platform.player.pos_y =
        max_f(
            min_f(
                global_game_context.platform.player.pos_y + dy,
                1.0f
            ),
            0.0f
        );

    global_game_context.platform.player.force =
        max_f(
            global_game_context.platform.player.force - SNOWMAN_DOWN_FORCE_STEP,
            SNOWMAN_MIN_FORCE
        );
}

void enter_perspective()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    gluPerspective(
        45,
        get_ar(),
        0.1,
        1000
    );

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void quit_perspective()
{
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void display_obstacle(game_context_t::platform_t::tree_t* tree)
{
    enter_perspective();

    glPushMatrix();
    //glRotatef(15, 1.0, 0, 0);

    glTranslatef(
        tree->pos_x * glutGet(GLUT_WINDOW_WIDTH) - glutGet(GLUT_WINDOW_HEIGHT) / 2.0,
        tree->pos_y * glutGet(GLUT_WINDOW_HEIGHT) / 4,
        -400
    );

    glPushMatrix();
    glScalef(20.0, 20.0, 20.0);
    circulartree(
        !tree->broken ? 1.0 : 0.6
    );
    glPopMatrix();

    glPopMatrix();

    quit_perspective();
}

void display_player(game_context_t::platform_t::player_t* player)
{
    enter_perspective();

    glPushMatrix();
    //glRotatef(15, 1.0, 0, 0);

    glTranslatef(
        0,
        player->pos_y * glutGet(GLUT_WINDOW_HEIGHT) / 4,
        -400
    );

    glPushMatrix();
    glScalef(10.0, 10.0, 10.0);
    draw_snowman(
        1.0 * (SNOWMAN_MAX_LIVES - player->lives) / SNOWMAN_MAX_LIVES
    );
    glPopMatrix();

    glPopMatrix();

    quit_perspective();
}

float get_ar()
{
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    return 1.0 * w / h;
}

void reset_game()
{
    global_game_context.platform.state = 0;
    global_game_context.intro_screen.state = 0;
    global_game_context.end_screen.state = 0;
}

void render_platform()
{
    enter_perspective();


    glPushMatrix();
    glTranslatef(0, -40, -400);
    glColor3f(
        1.0, 0.0, 0.0
    );
    glRotatef(-170, 1.0, 0, 0);
    const float platform_length = 1.0 * 800 * get_ar();
    const float platform_width = 200;
    glNormal3f(0, 1, 0);
    glColor3f(0.9, 0.6, 0.8);
    glBegin(GL_QUADS);
    glVertex3f(-platform_length / 2, 0, -platform_width / 2);
    glVertex3f(platform_length / 2, 0, -platform_width / 2);
    glVertex3f(platform_length / 2, 0, platform_width / 2);
    glVertex3f(-platform_length / 2, 0, platform_width / 2);
    glEnd();
    glPopMatrix();

    quit_perspective();

    if (global_game_context.platform.state == 0)
    {
        reinit_obstacles();
        global_game_context.platform.state = 1;
        global_game_context.platform.player.pos_y = 0.5;
        global_game_context.platform.player.force = 0;
        global_game_context.platform.player.lives = SNOWMAN_MAX_LIVES;
        global_game_context.platform.player.score = 0;
    }

    if (global_game_context.platform.state == 1)
    {
        populate_obstacles();
        update_player_energy();
        update_platform_state();
        display_obstacles();
        display_player(&global_game_context.platform.player);
    }
}


float move_unit = 0.02f;
void keyboardown(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_RIGHT:
        posX += move_unit;
        break;
    case GLUT_KEY_LEFT:
        posX -= move_unit;
        break;
    case GLUT_KEY_UP:
        posY += move_unit;
        break;
    case GLUT_KEY_DOWN:
        posY -= move_unit;
        break;
    default:
        break;
    }
    if (posX == sx1 || posX == sx2) {

        (sx1 -= 0.02);sy1 += 0.06;
        sx2 = 0.02;
        sy2 += 0.08;
        sx3 = 0.04;
        sy3 += 0.04;

    }

    glutPostRedisplay();
}

// Setup the rendering context
void init(void)
{
}

void timer_callback(int)
{
    ++global_game_step;

    glutPostRedisplay();

    glutTimerFunc(TIMER_PERIOD, timer_callback, 0x13);
}

void initRendering()
{
    reset_game();

    global_game_state = game_state_t::BEGINNING;
}

void keyPressed(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)
    {
    }
}

void render_text(game_context_t::splash_t* ctx)
{
#if 0
    fprintf(
        stderr,
        "render_text: [s:%d:ws:%d:cs:%d]\n",
        ctx->state,
        ctx->wait_steps,
        ctx->current_symbol
    );
#endif

    if (
        ctx->state == 0
        )
    {
        ctx->string_length = glutGet(GLUT_WINDOW_WIDTH) / 5;

        ctx->current_symbol = 0;
        ctx->state = 1;
    }

    if (
        ctx->state == 1
        )
    {
        ctx->wait_steps = rand() % 5 + 1;
        ctx->state = 2;
    }

    if (
        ctx->state == 2 &&
        ctx->wait_steps > 0

        )
    {
        --ctx->wait_steps;
    }

    if (
        ctx->state == 2 &&
        ctx->wait_steps == 0
        )
    {
        ctx->state = 3;
    }

    if (
        ctx->state == 3 ||
        ctx->state == 2
        )
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(
            0,
            glutGet(GLUT_WINDOW_WIDTH),
            0,
            glutGet(GLUT_WINDOW_HEIGHT),
            -10,
            10000
        );
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(
            (
                glutGet(GLUT_WINDOW_WIDTH) -
                min_f<int>(
                (int)strlen(ctx->msg) * 3,
                    glutGet(GLUT_WINDOW_WIDTH)
                    )
                ) / 2,
            glutGet(GLUT_WINDOW_HEIGHT) / 2,
            0
        );

        glRasterPos2i(
            0,
            0
        );
        for (
            int i = max_f(0, ctx->current_symbol - ctx->string_length);
            i < ctx->current_symbol;
            ++i
            )
        {
            glColor3f(
                1.0, 1.0, 1.0
            );

            glutBitmapCharacter(
                GLUT_BITMAP_TIMES_ROMAN_10,
                (int)ctx->msg[i]
            );
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    if (
        ctx->state == 3
        )
    {
        ctx->state = 1;
        ctx->current_symbol =
            (
                ctx->current_symbol + 1
                ) % strlen(
                    ctx->msg
                );
    }
}

void display_end_game()
{
    game_context_t::splash_t* t1 = &global_game_context.end_screen;

    if (t1->state == 0)
    {
        snprintf(
            t1->msg,
            sizeof(t1->msg),
            "The game is over. Your score is %d."
            "Press Enter to restart the game."
            "..................................."
            "..................................."
            "Author: praveen "
            "Year: 1446. "
            "License: GPLv3. "
            "...............",
            global_game_context.platform.player.score
        );
    }

    render_text(t1);
}

void display_intro()
{
    game_context_t::splash_t* t1 = &global_game_context.intro_screen;

    if (
        t1->state == 0
        )
    {
        snprintf(
            t1->msg,
            sizeof(t1->msg),
            "In the beginning there was nothing. "
            "Except for flying snowmans. "
            "And trees. "
            "Snowmans were running among trees..."
            "Space bar will let you fly."
            "Press Enter to begin the game....................."
        );
    }

    render_text(t1);

}

void reshape_callback(int w, int h)
{
    glViewport(0, 0, w, h);
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutCreateWindow("The Flying Snowman");
    initRendering();
    glutReshapeFunc(reshape_callback);
    glutDisplayFunc(display);
    glutKeyboardFunc(keys);
    glutSpecialFunc(keyboardown);
    glutTimerFunc(TIMER_PERIOD, timer_callback, 0x13);

    init();
    glutMainLoop();
    return 0;
}
