#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>

#include <libwacom/libwacom.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include <SDL2/SDL.h>

#include <X11/extensions/XInput.h>

#include <libinput.h>

void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer with current clearing color

    // Define shapes enclosed within a pair of glBegin and glEnd
    glBegin(GL_QUADS);           // Each set of 4 vertices form a quad
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex2f(-0.8f, 0.1f);     // Define vertices in counter-clockwise (CCW) order
    glVertex2f(-0.2f, 0.1f);     //  so that the normal (front-face) is facing you
    glVertex2f(-0.2f, 0.7f);
    glVertex2f(-0.8f, 0.7f);

    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glVertex2f(-0.7f, -0.6f);
    glVertex2f(-0.1f, -0.6f);
    glVertex2f(-0.1f, 0.0f);
    glVertex2f(-0.7f, 0.0f);

    glColor3f(0.2f, 0.2f, 0.2f); // Dark Gray
    glVertex2f(-0.9f, -0.7f);
    glColor3f(1.0f, 1.0f, 1.0f); // White
    glVertex2f(-0.5f, -0.7f);
    glColor3f(0.2f, 0.2f, 0.2f); // Dark Gray
    glVertex2f(-0.5f, -0.3f);
    glColor3f(1.0f, 1.0f, 1.0f); // White
    glVertex2f(-0.9f, -0.3f);
    glEnd();

    glBegin(GL_TRIANGLES);       // Each set of 3 vertices form a triangle
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    glVertex2f(0.1f, -0.6f);
    glVertex2f(0.7f, -0.6f);
    glVertex2f(0.4f, -0.1f);

    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex2f(0.3f, -0.4f);
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glVertex2f(0.9f, -0.4f);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    glVertex2f(0.6f, -0.9f);
    glEnd();

    glBegin(GL_POLYGON);         // These vertices form a closed polygon
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    glVertex2f(0.4f, 0.2f);
    glVertex2f(0.6f, 0.2f);
    glVertex2f(0.7f, 0.4f);
    glVertex2f(0.6f, 0.6f);
    glVertex2f(0.4f, 0.6f);
    glVertex2f(0.3f, 0.4f);
    glEnd();

    //glFlush();  // Render now
}

static int event_devices_only(const struct dirent *dir)
{
    return strncmp("event", dir->d_name, 5) == 0;
}

int main(int argc, char *argv[])
{
    //XDeviceInfo* deviceInfo = XListInputDevices(m_display, &deviceCount);

    WacomDeviceDatabase *db;
    WacomDevice *device;
    WacomError *error;
    db = libwacom_database_new();
    error = libwacom_error_new();

    WacomDevice **devices = libwacom_list_devices_from_database(db, error);
    if (devices != NULL)
    {
        for (int i = 0; devices[i] != NULL; i++)
        {
            //libwacom_print_device_description(STDOUT_FILENO, devices[i]);
        }
    }

    struct dirent **namelist = NULL;
    int i = scandir("/dev/input", &namelist, event_devices_only, alphasort);

    if (i < 0 || i == 0)
    {
        fprintf(stderr, "Failed to find any devices.\n");
        return 2;
    }

    while (i--)
    {
        char fname[PATH_MAX];
        snprintf(fname, sizeof(fname), "/dev/input/%s", namelist[i]->d_name);

        WacomDevice *dev = libwacom_new_from_path(db, fname, WFALLBACK_GENERIC, NULL);
        if (!dev)
        {
            continue;
        }

        printf("device %s : %s\n", fname, libwacom_get_name(dev));
        //libwacom_print_device_description(STDOUT_FILENO, dev);

        libwacom_destroy(dev);
    }

    device = libwacom_new_from_path(db, "/dev/input/event18", WFALLBACK_GENERIC, error);
    if (!device)
        return -1; // should check for error here
    if (libwacom_is_builtin(device))
        printf("This is a built-in device\n");

    int width = libwacom_get_width(device);
    int height = libwacom_get_height(device);
    int hasStylus = libwacom_has_stylus(device);

    printf("w:%d h:%d s:%d\n", width, height, hasStylus);

    int nbStylus = 0;
    int *stylusIds = libwacom_get_supported_styli(device, &nbStylus);
    printf("nb stylus:%d\n", nbStylus);

    for (int i = 0; i < nbStylus; i++)
    {
        WacomStylus *stylus = libwacom_stylus_get_for_id(db, stylusIds[i]);
        printf("stylus : %s\n", libwacom_stylus_get_name(stylus));
    }

    SDL_bool quit;

    SDL_Window *window;
    SDL_GLContext glContext;
    SDL_Event sdlEvent;

    quit = SDL_FALSE;

    //Use OpenGL 3.1 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Initialize video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        // Display error message
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
    else
    {
        // Create window
        window = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (window == NULL)
        {
            // Display error message
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return 0;
        }
        else
        {
            // Create OpenGL context
            glContext = SDL_GL_CreateContext(window);

            if (glContext == NULL)
            {
                // Display error message
                printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
                return 0;
            }
            else
            {
                // Initialize glew
                glewInit();
            }
        }
    }

    // Game loop
    float curX = 0.9f;
    float curY = -0.4f;

    while (!quit)
    {
        while (SDL_PollEvent(&sdlEvent) != 0)
        {
            // Esc button is pressed
            switch (sdlEvent.type)
            {
            case SDL_QUIT:
                quit = SDL_TRUE;
                break;

            case SDL_FINGERMOTION:
            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
                //case SDL_MOUSEMOTION:
                {
                    curX = sdlEvent.tfinger.x;
                    curY = sdlEvent.tfinger.y;
                    //printf("Current finger position is: (%f, %f, %f)\n", sdlEvent.tfinger.x, sdlEvent.tfinger.y, sdlEvent.tfinger.pressure);
                }
                break;

            case SDL_MOUSEMOTION:
                //printf("Current mouse position is: (%d, %d)\n", sdlEvent.motion.x, sdlEvent.motion.y);
                curX = sdlEvent.motion.x / 400.0f - 1.0f;
                curY = -(sdlEvent.motion.y / 300.0f - 1.0f);
                break;
            }
        }

        // Set background color as cornflower blue
        glClearColor(0.39f, 0.58f, 0.93f, 1.f);
        // Clear color buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        glVertex2f(0.0f, -0.0f);
        glColor3f(0.0f, 1.0f, 0.0f); // Green
        glVertex2f(curX, curY);
        glColor3f(0.0f, 0.0f, 1.0f); // Blue
        glVertex2f(0.6f, -0.9f);
        glEnd();

        //display();

        // Update window with OpenGL rendering
        SDL_GL_SwapWindow(window);
    }

    //Destroy window
    SDL_DestroyWindow(window);
    window = NULL;

    //Quit SDL subsystems
    SDL_Quit();

    libwacom_destroy(device);
    libwacom_database_destroy(db);

    return 0;
}

int oldmain(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (win == NULL)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == NULL)
    {
        SDL_DestroyWindow(win);
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Event e;
    SDL_bool quit = SDL_FALSE;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = SDL_TRUE;
            }
            if (e.type == SDL_KEYDOWN)
            {
                quit = SDL_TRUE;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                quit = SDL_TRUE;
            }
        }
        //Render the scene
        SDL_RenderClear(ren);
        //renderTexture(image, renderer, x, y);
        SDL_RenderPresent(ren);
    }

    //SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}