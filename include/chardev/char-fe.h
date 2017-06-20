#ifndef QEMU_CHAR_FE_H
#define QEMU_CHAR_FE_H

#include "chardev/char.h"

typedef void IOEventHandler(void *opaque, int event);

/* This is the backend as seen by frontend, the actual backend is
 * Chardev */
struct CharBackend {
    Chardev *chr;
    IOEventHandler *chr_event;
    IOCanReadHandler *chr_can_read;
    IOReadHandler *chr_read;
    void *opaque;
    int tag;
    int fe_open;
};

/**
 * @qemu_chr_fe_init:
 *
 * Initializes a front end for the given CharBackend and
 * Chardev. Call qemu_chr_fe_deinit() to remove the association and
 * release the driver.
 *
 * Returns: false on error.
 */
bool qemu_chr_fe_init(CharBackend *b, Chardev *s, Error **errp);

/**
 * @qemu_chr_fe_deinit:
 * @b: a CharBackend
 * @del: if true, delete the chardev backend
*
 * Dissociate the CharBackend from the Chardev.
 *
 * Safe to call without associated Chardev.
 */
void qemu_chr_fe_deinit(CharBackend *b, bool del);

/**
 * @qemu_chr_fe_get_driver:
 *
 * Returns the driver associated with a CharBackend or NULL if no
 * associated Chardev.
 */
Chardev *qemu_chr_fe_get_driver(CharBackend *be);

/**
 * @qemu_chr_fe_set_handlers:
 * @b: a CharBackend
 * @fd_can_read: callback to get the amount of data the frontend may
 *               receive
 * @fd_read: callback to receive data from char
 * @fd_event: event callback
 * @opaque: an opaque pointer for the callbacks
 * @context: a main loop context or NULL for the default
 * @set_open: whether to call qemu_chr_fe_set_open() implicitely when
 * any of the handler is non-NULL
 *
 * Set the front end char handlers. The front end takes the focus if
 * any of the handler is non-NULL.
 *
 * Without associated Chardev, nothing is changed.
 */
void qemu_chr_fe_set_handlers(CharBackend *b,
                              IOCanReadHandler *fd_can_read,
                              IOReadHandler *fd_read,
                              IOEventHandler *fd_event,
                              void *opaque,
                              GMainContext *context,
                              bool set_open);

/**
 * @qemu_chr_fe_take_focus:
 *
 * Take the focus (if the front end is muxed).
 *
 * Without associated Chardev, nothing is changed.
 */
void qemu_chr_fe_take_focus(CharBackend *b);

/**
 * @qemu_chr_fe_accept_input:
 *
 * Notify that the frontend is ready to receive data
 */
void qemu_chr_fe_accept_input(CharBackend *be);

/**
 * @qemu_chr_fe_disconnect:
 *
 * Close a fd accpeted by character backend.
 * Without associated Chardev, do nothing.
 */
void qemu_chr_fe_disconnect(CharBackend *be);

/**
 * @qemu_chr_fe_wait_connected:
 *
 * Wait for characted backend to be connected, return < 0 on error or
 * if no assicated Chardev.
 */
int qemu_chr_fe_wait_connected(CharBackend *be, Error **errp);

/**
 * @qemu_chr_fe_set_echo:
 *
 * Ask the backend to override its normal echo setting.  This only really
 * applies to the stdio backend and is used by the QMP server such that you
 * can see what you type if you try to type QMP commands.
 * Without associated Chardev, do nothing.
 *
 * @echo true to enable echo, false to disable echo
 */
void qemu_chr_fe_set_echo(CharBackend *be, bool echo);

/**
 * @qemu_chr_fe_set_open:
 *
 * Set character frontend open status.  This is an indication that the
 * front end is ready (or not) to begin doing I/O.
 * Without associated Chardev, do nothing.
 */
void qemu_chr_fe_set_open(CharBackend *be, int fe_open);

/**
 * @qemu_chr_fe_printf:
 *
 * Write to a character backend using a printf style interface.  This
 * function is thread-safe. It does nothing without associated
 * Chardev.
 *
 * @fmt see #printf
 */
void qemu_chr_fe_printf(CharBackend *be, const char *fmt, ...)
    GCC_FMT_ATTR(2, 3);

/**
 * @qemu_chr_fe_add_watch:
 *
 * If the backend is connected, create and add a #GSource that fires
 * when the given condition (typically G_IO_OUT|G_IO_HUP or G_IO_HUP)
 * is active; return the #GSource's tag.  If it is disconnected,
 * or without associated Chardev, return 0.
 *
 * @cond the condition to poll for
 * @func the function to call when the condition happens
 * @user_data the opaque pointer to pass to @func
 *
 * Returns: the source tag
 */
guint qemu_chr_fe_add_watch(CharBackend *be, GIOCondition cond,
                            GIOFunc func, void *user_data);

/**
 * @qemu_chr_fe_write:
 *
 * Write data to a character backend from the front end.  This function
 * will send data from the front end to the back end.  This function
 * is thread-safe.
 *
 * @buf the data
 * @len the number of bytes to send
 *
 * Returns: the number of bytes consumed (0 if no assicated Chardev)
 */
int qemu_chr_fe_write(CharBackend *be, const uint8_t *buf, int len);

/**
 * @qemu_chr_fe_write_all:
 *
 * Write data to a character backend from the front end.  This function will
 * send data from the front end to the back end.  Unlike @qemu_chr_fe_write,
 * this function will block if the back end cannot consume all of the data
 * attempted to be written.  This function is thread-safe.
 *
 * @buf the data
 * @len the number of bytes to send
 *
 * Returns: the number of bytes consumed (0 if no assicated Chardev)
 */
int qemu_chr_fe_write_all(CharBackend *be, const uint8_t *buf, int len);

/**
 * @qemu_chr_fe_read_all:
 *
 * Read data to a buffer from the back end.
 *
 * @buf the data buffer
 * @len the number of bytes to read
 *
 * Returns: the number of bytes read (0 if no assicated Chardev)
 */
int qemu_chr_fe_read_all(CharBackend *be, uint8_t *buf, int len);

/**
 * @qemu_chr_fe_ioctl:
 *
 * Issue a device specific ioctl to a backend.  This function is thread-safe.
 *
 * @cmd see CHR_IOCTL_*
 * @arg the data associated with @cmd
 *
 * Returns: if @cmd is not supported by the backend or there is no
 *          associated Chardev, -ENOTSUP, otherwise the return
 *          value depends on the semantics of @cmd
 */
int qemu_chr_fe_ioctl(CharBackend *be, int cmd, void *arg);

/**
 * @qemu_chr_fe_get_msgfd:
 *
 * For backends capable of fd passing, return the latest file descriptor passed
 * by a client.
 *
 * Returns: -1 if fd passing isn't supported or there is no pending file
 *          descriptor.  If a file descriptor is returned, subsequent calls to
 *          this function will return -1 until a client sends a new file
 *          descriptor.
 */
int qemu_chr_fe_get_msgfd(CharBackend *be);

/**
 * @qemu_chr_fe_get_msgfds:
 *
 * For backends capable of fd passing, return the number of file received
 * descriptors and fills the fds array up to num elements
 *
 * Returns: -1 if fd passing isn't supported or there are no pending file
 *          descriptors.  If file descriptors are returned, subsequent calls to
 *          this function will return -1 until a client sends a new set of file
 *          descriptors.
 */
int qemu_chr_fe_get_msgfds(CharBackend *be, int *fds, int num);

/**
 * @qemu_chr_fe_set_msgfds:
 *
 * For backends capable of fd passing, set an array of fds to be passed with
 * the next send operation.
 * A subsequent call to this function before calling a write function will
 * result in overwriting the fd array with the new value without being send.
 * Upon writing the message the fd array is freed.
 *
 * Returns: -1 if fd passing isn't supported or no associated Chardev.
 */
int qemu_chr_fe_set_msgfds(CharBackend *be, int *fds, int num);

#endif /* QEMU_CHAR_FE_H */