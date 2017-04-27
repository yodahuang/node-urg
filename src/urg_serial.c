/*!
  \file
  \brief Serial communications
  \author Satofumi KAMIMURA

  $Id: urg_serial.c,v e5d1719877a2 2015/05/07 04:12:14 jun $
*/

#include "urg_serial.h"


enum {
    False = 0,
    True,
};


/*!
  \file
   Serial communications in Linux
  \author Satofumi KAMIMURA

  $Id: urg_serial_linux.c,v c5747add6615 2015/05/07 03:18:34 alexandr $
*/

#include "urg_ring_buffer.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>


enum {
    INVALID_FD = -1,
};


static void serial_initialize(urg_serial_t *serial)
{
    serial->fd = INVALID_FD;
    serial->has_last_ch = False;

    ring_initialize(&serial->ring, serial->buffer, RING_BUFFER_SIZE_SHIFT);
}


static void serial_clear(urg_serial_t* serial)
{
    tcdrain(serial->fd);
    tcflush(serial->fd, TCIOFLUSH);
    ring_clear(&serial->ring);
    serial->has_last_ch = False;
}


int serial_open(urg_serial_t *serial, const char *device, long baudrate)
{
    int flags = 0;
    int ret = 0;

    serial_initialize(serial);

#ifndef URG_MAC_OS
    enum { O_EXLOCK = 0x0 }; /*  Not used in Linux, used as dummy */
#endif
    serial->fd = open(device, O_RDWR | O_EXLOCK | O_NONBLOCK | O_NOCTTY);
    if (serial->fd < 0) {
        /*  Connection failed */
        //strerror_r(errno, serial->error_string, ERROR_MESSAGE_SIZE);
        return -1;
    }

    flags = fcntl(serial->fd, F_GETFL, 0);
    fcntl(serial->fd, F_SETFL, flags & ~O_NONBLOCK);

    /*  Initializes serial communication  */
    tcgetattr(serial->fd, &serial->sio);
    serial->sio.c_iflag = 0;
    serial->sio.c_oflag = 0;
    serial->sio.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    serial->sio.c_cflag |= CS8 | CREAD | CLOCAL;
    serial->sio.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);

    serial->sio.c_cc[VMIN] = 0;
    serial->sio.c_cc[VTIME] = 0;

    ret = serial_set_baudrate(serial, baudrate);
    if (ret < 0) {
        return ret;
    }

    /*  Initializes serial control structures */
    serial->has_last_ch = False;

    return 0;
}


void serial_close(urg_serial_t *serial)
{
    if (serial->fd >= 0) {
        close(serial->fd);
        serial->fd = INVALID_FD;
    }
}


int serial_set_baudrate(urg_serial_t *serial, long baudrate)
{
    long baudrate_value = -1;

    switch (baudrate) {
    case 4800:
        baudrate_value = B4800;
        break;

    case 9600:
        baudrate_value = B9600;
        break;

    case 19200:
        baudrate_value = B19200;
        break;

    case 38400:
        baudrate_value = B38400;
        break;

    case 57600:
        baudrate_value = B57600;
        break;

    case 115200:
        baudrate_value = B115200;
        break;

    default:
        return -1;
    }

    /*  Changes the baudrate */
    cfsetospeed(&serial->sio, baudrate_value);
    cfsetispeed(&serial->sio, baudrate_value);
    tcsetattr(serial->fd, TCSADRAIN, &serial->sio);
    serial_clear(serial);

    return 0;
}


int serial_write(urg_serial_t *serial, const char *data, int size)
{
    if (serial->fd == INVALID_FD) {
        return -1;
    }
    return write(serial->fd, data, size);
}


static int wait_receive(urg_serial_t* serial, int timeout)
{
    fd_set rfds;
    struct timeval tv;

    // Configures the timeout
    FD_ZERO(&rfds);
    FD_SET(serial->fd, &rfds);

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    if (select(serial->fd + 1, &rfds, NULL, NULL,
               (timeout < 0) ? NULL : &tv) <= 0) {
        /*  Timeout occurred */
        return 0;
    }
    return 1;
}


static int internal_receive(char data[], int data_size_max,
                            urg_serial_t* serial, int timeout)
{
    int filled = 0;

    if (data_size_max <= 0) {
        return 0;
    }

    while (filled < data_size_max) {
        int require_n;
        int read_n;

        if (! wait_receive(serial, timeout)) {
            break;
        }

        require_n = data_size_max - filled;
        read_n = read(serial->fd, &data[filled], require_n);
        if (read_n <= 0) {
            /*  Read error, returns all the data up to now */
            break;
        }
        filled += read_n;
    }
    return filled;
}


int serial_read(urg_serial_t *serial, char *data, int max_size, int timeout)
{
    int buffer_size;
    int read_n;
    int filled = 0;

    if (max_size <= 0) {
        return 0;
    }

    /*  If there is a single character return it */
    if (serial->has_last_ch != False) {
        data[0] = serial->last_ch;
        serial->has_last_ch = False;
        ++filled;
    }
    if (serial->fd == INVALID_FD) {
        if (filled > 0) {
            return filled;
        } else {
            return -1;
        }
    }

    buffer_size = ring_size(&serial->ring);
    read_n = max_size - filled;
    if (buffer_size < read_n) {
        // Reads data if there is space in the ring buffer
        char buffer[RING_BUFFER_SIZE];
        int n = internal_receive(buffer,
                                 ring_capacity(&serial->ring) - buffer_size,
                                 serial, 0);
        if (n > 0) {
            ring_write(&serial->ring, buffer, n);
            buffer_size += n;
        }
    }

    // Returns the data stored in the ring buffer
    if (read_n > buffer_size) {
        read_n = buffer_size;
    }
    if (read_n > 0) {
        ring_read(&serial->ring, &data[filled], read_n);
        filled += read_n;
    }

    // Reads data within the given timeout
    filled += internal_receive(&data[filled], max_size - filled,
                               serial, timeout);
    return filled;
}


// Checks wheter is is a EOL character
static int is_linefeed(const char ch)
{
    return ((ch == '\r') || (ch == '\n')) ? 1 : 0;
}


static void serial_ungetc(urg_serial_t *serial, char ch)
{
    serial->has_last_ch = True;
    serial->last_ch = ch;
}


int serial_readline(urg_serial_t *serial, char *data, int max_size, int timeout)
{
    /* Reads and evaluates 1 character at a time */
    int filled = 0;
    int is_timeout = 0;

    while (filled < max_size) {
        char recv_ch;
        int n = serial_read(serial, &recv_ch, 1, timeout);
        if (n <= 0) {
            is_timeout = 1;
            break;
        } else if (is_linefeed(recv_ch)) {
            break;
        }
        data[filled++] = recv_ch;
    }
    if (filled >= max_size) {
        --filled;
        serial_ungetc(serial, data[filled]);
    }
    data[filled] = '\0';

    if ((filled == 0) && is_timeout) {
        return -1;
    } else {
        //fprintf(stderr, "%s\n", data);
        return filled;
    }
}
