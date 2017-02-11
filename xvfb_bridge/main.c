#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XShm.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <time.h>

#include "linux/lux.h"
#include "log.h"

enum loglevel loglevel = LOGLEVEL_INFO;

Window Select_Window(Display * dpy, int screen) {
  int status;
  Cursor cursor;
  XEvent event;
  Window target_win = None, root = RootWindow(dpy,screen);
  int buttons = 0;

  /* Make the target cursor */
  cursor = XCreateFontCursor(dpy, XC_crosshair);

  /* Grab the pointer using target cursor, letting it room all over */
  status = XGrabPointer(dpy, root, False,
			ButtonPressMask|ButtonReleaseMask, GrabModeSync,
			GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) FAIL("Can't grab the mouse.");

  /* Let the user select a window... */
  while ((target_win == None) || (buttons != 0)) {
    /* allow one more event */
    XAllowEvents(dpy, SyncPointer, CurrentTime);
    XWindowEvent(dpy, root, ButtonPressMask|ButtonReleaseMask, &event);
    switch (event.type) {
    case ButtonPress:
      if (target_win == None) {
	target_win = event.xbutton.subwindow; /* window selected */
	if (target_win == None) target_win = root;
      }
      buttons++;
      break;
    case ButtonRelease:
      if (buttons > 0) /* there may have been some down before we started */
	buttons--;
       break;
    }
  } 

  XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */

  return(target_win);
}

Window Window_With_Name(Display * dpy, Window top, char * name) {
	Window *children, dummy;
	unsigned int nchildren;
	int i;
	Window w=0;
	char *window_name;

	if (XFetchName(dpy, top, &window_name) && !strcmp(window_name, name))
	  return(top);

	if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
	  return(0);

	for (i=0; i<(int)nchildren; i++) {
		w = Window_With_Name(dpy, children[i], name);
		if (w)
		  break;
	}
	if (children) XFree ((char *)children);
	return(w);
}

struct grabber {
    // X11 things
    Display * display;
    int screen;
    Window window;
    XWindowAttributes window_attrs;
    XImage *image;
    XShmSegmentInfo shminfo;

    // Per-pixel
    size_t size;
    double * xs;
    double * ys;
    XColor * colors;
};

int grabber_init(struct grabber * grabber, size_t size) {
    memset(grabber, 0, sizeof *grabber);

    grabber->size = size;
    grabber->xs = calloc(sizeof *grabber->xs, size);
    ASSERT(grabber->xs != NULL);
    grabber->ys = calloc(sizeof *grabber->ys, size);
    ASSERT(grabber->ys != NULL);
    grabber->colors = calloc(sizeof *grabber->colors, size);
    ASSERT(grabber->colors != NULL);

    grabber->display = XOpenDisplay(NULL);
    ASSERT(grabber->display != NULL, "Unable to open display");

    grabber->screen = DefaultScreen(grabber->display);
    INFO("Select window...");
    grabber->window = Select_Window(grabber->display, grabber->screen);

    int rc = XGetWindowAttributes(grabber->display, grabber->window, &grabber->window_attrs);
    ASSERT(rc == 1, "Unable to get window attributes (%d)", rc);

    // Setup shm
    XImage *img = XShmCreateImage(
            grabber->display,
            grabber->window_attrs.visual,
            grabber->window_attrs.depth,
            ZPixmap,
            NULL,
            &grabber->shminfo,
            grabber->window_attrs.width,
            grabber->window_attrs.height);
    grabber->shminfo.shmid = shmget(IPC_PRIVATE, img->bytes_per_line * img->height, IPC_CREAT | 0777);
    ASSERT(grabber->shminfo.shmid != -1);

    grabber->shminfo.shmaddr = img->data = shmat(grabber->shminfo.shmid, 0, 0);
    grabber->shminfo.readOnly = 0;

    rc = XShmAttach(grabber->display, &grabber->shminfo);
    ASSERT(rc);

    grabber->image = img;

    return 0;
}

int grabber_grab(struct grabber * grabber) {
    //int x = 0, y = 0;
    int w = grabber->window_attrs.width, h = grabber->window_attrs.height;
    //XImage * image = XGetImage(grabber->display, grabber->window, x, y, w, h, AllPlanes, XYPixmap);
    //ASSERT(image != NULL, "Unable to capture image");
    int rc = XShmGetImage(grabber->display, grabber->window, grabber->image, 0, 0, AllPlanes);
    ASSERT(rc);
    XImage * image = grabber->image;

    memset(grabber->colors, 0, grabber->size * sizeof *grabber->colors);
    for (size_t i = 0; i < grabber->size; i++) {
        int px = w * grabber->xs[i];
        int py = h * grabber->xs[i];
        grabber->colors[i].pixel = XGetPixel(image, px, py);
    }

    rc = XQueryColors(grabber->display, grabber->window_attrs.colormap, grabber->colors, grabber->size);
    ASSERT(rc == 1, "Unable to query colors (%d)", rc);

    //XFree(image);

    return 0;
}

int grabber_grabsparse(struct grabber * grabber) {
    int w = grabber->window_attrs.width, h = grabber->window_attrs.height;
    memset(grabber->colors, 0, grabber->size * sizeof *grabber->colors);

    XImage * image = XGetImage(grabber->display, grabber->window, 0, 0, 1, 1, AllPlanes, XYPixmap);
    ASSERT(image != NULL, "Unable to capture image");

    for (size_t i = 0; i < grabber->size; i++) {
        int px = w * grabber->xs[i];
        int py = h * grabber->xs[i];

        void * ri = XGetSubImage(grabber->display, grabber->window, px, py, 1, 1, AllPlanes, XYPixmap, image, 0, 0);
        ASSERT(ri != NULL);

        grabber->colors[i].pixel = XGetPixel(image, 0, 0);

    }
    XFree(image);

    int rc = XQueryColors(grabber->display, grabber->window_attrs.colormap, grabber->colors, grabber->size);
    ASSERT(rc == 1, "Unable to query colors (%d)", rc);

    return 0;
}

int main(void) {
    struct grabber grabber;
    size_t n_pixels = 1000;
    grabber_init(&grabber, n_pixels);

    for (size_t i = 0; i < n_pixels; i++) {
        // Diagonal across the screen
        grabber.xs[1] = 1.0 / (double) (n_pixels - 1);
        grabber.ys[1] = 1.0 / (double) (n_pixels - 1);
    }

    int fd = lux_network_open("127.0.0.1", 1365);
    if (fd < 0)
        PFAIL("Unable to open lux socket");

    while (1) {
        struct lux_packet packet = {
            .destination = -1,
            .command = CMD_FRAME,
            .payload_length = n_pixels * 3,
        };
        grabber_grabsparse(&grabber);
        uint8_t * p = packet.payload;
        for (size_t i = 0; i < n_pixels; i++) {
            *p++ = grabber.colors[i].red;
            *p++ = grabber.colors[i].green;
            *p++ = grabber.colors[i].blue;
        }
        int rc = lux_write(fd, &packet);
        if (rc < 0)
            PFAIL("Unable to write lux packet");
    }

    /*
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    grabber_grab(&grabber);
    grabber_grab(&grabber);
    grabber_grab(&grabber);
    grabber_grab(&grabber);
    grabber_grab(&grabber);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    INFO("Grabbing 5 samples took %lu us", (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000);

    XColor c = grabber.colors[0];
    INFO("Got color: rgb:%d,%d,%d", c.red >> 8, c.green >> 8, c.blue >> 8);
    c = grabber.colors[1];
    INFO("Got color: rgb:%d,%d,%d", c.red >> 8, c.green >> 8, c.blue >> 8);
    */

    return 0;
}
