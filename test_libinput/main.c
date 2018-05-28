#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <getopt.h>
#include <sys/stat.h>
#include <libudev.h>
#include <stdbool.h>

#include <libinput.h>

LIBINPUT_ATTRIBUTE_PRINTF(3, 0)
static void
log_handler(struct libinput *li,
            enum libinput_log_priority priority,
            const char *format,
            va_list args)
{
    static int is_tty = -1;

    /*if (is_tty == -1)
        is_tty = isatty(STDOUT_FILENO);

    if (is_tty)
    {
        if (priority >= LIBINPUT_LOG_PRIORITY_ERROR)
            printf(ANSI_RED);
        else if (priority >= LIBINPUT_LOG_PRIORITY_INFO)
            printf(ANSI_HIGHLIGHT);
    }*/

    vprintf(format, args);

    //if (is_tty && priority >= LIBINPUT_LOG_PRIORITY_INFO)
    //    printf(ANSI_NORMAL);
}

static int
open_restricted(const char *path, int flags, void *user_data)
{
    bool *grab = user_data;
    int fd = open(path, flags);

    if (fd < 0)
        fprintf(stderr, "Failed to open %s (%s)\n",
                path, strerror(errno));
    //else if (*grab && ioctl(fd, EVIOCGRAB, (void *)1) == -1)
    //    fprintf(stderr, "Grab requested, but failed for %s (%s)\n",
    //            path, strerror(errno));

    return fd < 0 ? -errno : fd;
}

static void
close_restricted(int fd, void *user_data)
{
    close(fd);
}

static const struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

static struct libinput *
tools_open_device(const char *path, bool verbose, bool grab)
{
    struct libinput_device *device;
    struct libinput *li;

    li = libinput_path_create_context(&interface, &grab);
    if (!li)
    {
        fprintf(stderr, "Failed to initialize context from %s\n", path);
        return NULL;
    }

    if (verbose)
    {
        libinput_log_set_handler(li, log_handler);
        libinput_log_set_priority(li, LIBINPUT_LOG_PRIORITY_DEBUG);
    }

    device = libinput_path_add_device(li, path);
    if (!device)
    {
        fprintf(stderr, "Failed to initialized device %s\n", path);
        libinput_unref(li);
        li = NULL;
    }

    return li;
}

static void
handle_event_libinput(struct libinput *li)
{
    //struct window *w = libinput_get_user_data(li);
    struct libinput_event *ev;

    libinput_dispatch(li);

    while ((ev = libinput_get_event(li)))
    {
        //printf("handle libinput event\n");

        enum libinput_event_type eventType = libinput_event_get_type(ev);
        switch (eventType)
        {
        /*case LIBINPUT_EVENT_NONE:
            abort();
        case LIBINPUT_EVENT_DEVICE_ADDED:
        case LIBINPUT_EVENT_DEVICE_REMOVED:
            handle_event_device_notify(ev);
            break;*/
        /*case LIBINPUT_EVENT_POINTER_MOTION:
            handle_event_motion(ev, w);
            break;
        case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
            handle_event_absmotion(ev, w);
            break;
        case LIBINPUT_EVENT_TOUCH_DOWN:
        case LIBINPUT_EVENT_TOUCH_MOTION:
        case LIBINPUT_EVENT_TOUCH_UP:
            handle_event_touch(ev, w);
            break;
        case LIBINPUT_EVENT_POINTER_AXIS:
            handle_event_axis(ev, w);
            break;
        case LIBINPUT_EVENT_TOUCH_CANCEL:
        case LIBINPUT_EVENT_TOUCH_FRAME:
            break;
        case LIBINPUT_EVENT_POINTER_BUTTON:
            handle_event_button(ev, w);
            break;
        case LIBINPUT_EVENT_KEYBOARD_KEY:
            if (handle_event_keyboard(ev, w))
            {
                libinput_event_destroy(ev);
                gtk_main_quit();
                return FALSE;
            }
            break;
        case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
        case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
        case LIBINPUT_EVENT_GESTURE_SWIPE_END:
            handle_event_swipe(ev, w);
            break;
        case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
        case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
        case LIBINPUT_EVENT_GESTURE_PINCH_END:
            handle_event_pinch(ev, w);
            break;*/
        case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
        case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
        //case LIBINPUT_EVENT_TABLET_TOOL_TIP:
        case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
        {
            struct libinput_event_tablet_tool *tabletEvent = libinput_event_get_tablet_tool_event(ev);
            double x = libinput_event_tablet_tool_get_x(tabletEvent);
            double y = libinput_event_tablet_tool_get_y(tabletEvent);
            //double xTransformed = libinput_event_tablet_tool_get_x_transformed(tabletEvent);
            //double yTransformed = libinput_event_tablet_tool_get_y_traansformed(tabletEvent);
            double dx = libinput_event_tablet_tool_get_dx(tabletEvent);
            double dy = libinput_event_tablet_tool_get_dy(tabletEvent);
            double pressure = libinput_event_tablet_tool_get_pressure(tabletEvent);
            double distance = libinput_event_tablet_tool_get_distance(tabletEvent);
            double rotation = libinput_event_tablet_tool_get_rotation(tabletEvent);
            double tiltX = libinput_event_tablet_tool_get_tilt_x(tabletEvent);
            double tiltY = libinput_event_tablet_tool_get_tilt_y(tabletEvent);

            if (pressure > 0)
                printf("tablet event %d %f (%f), %f (%f), %f, %f, %f, %f;%f\n", eventType, x, dx, y, dy, pressure, distance, rotation, tiltX, tiltY);
            //handle_event_tablet(ev, w);
        }
        break;
            /*case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
        case LIBINPUT_EVENT_TABLET_PAD_RING:
        case LIBINPUT_EVENT_TABLET_PAD_STRIP:
            break;*/

        default:
            break;
        }

        libinput_event_destroy(ev);
        libinput_dispatch(li);
    }
}

int main(int argc, char *argv[])
{
    struct libinput *lib = tools_open_device("/dev/input/event18", true, false);

    while (true)
    {
        handle_event_libinput(lib);
    }

    libinput_unref(lib);

    return 0;
}