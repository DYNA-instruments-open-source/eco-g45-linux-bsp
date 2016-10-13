/******************************************************************************/
/*                                                                            */
/* File:        capture.c                                                     */
/*                                                                            */
/* Description: Module for Linux v4l2 capture access.                         */
/*                                                                            */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com              */
/*                                                                            */
/******************************************************************************/

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <assert.h>

/* Local include files */
#include "v4l2capture.h"
#include "framebuffer.h"
#include "format.h"

/* local defines */
#define N_BUFFERS 3

/* Structs */
struct capture_buffer {
    void *start;
    size_t length;
};

/* Types */
/* Global variables */
char *conf_device_name_cap  = "/dev/video0"; /* name of the standard
											    capture device*/
io_method_t conf_io_method = IO_METHOD_MMAP; /* standard I/O method to 
											    use */
unsigned int conf_frame_count = 100; /* default frame count */
unsigned int conf_frame_width = 640; /* default frame width */
unsigned int conf_frame_height = 480; /* default frame height */ 
unsigned int conf_pixelformat = V4L2_PIX_FMT_YUV420;

static int  fd_cap = -1; /* file descriptor of the framebuffer device */
struct capture_buffer *capture_buffers = NULL;
static unsigned int n_buffers = 0;

/* Local helper function declarations */
static error_t init_capture_read(unsigned int buffer_size);
static error_t init_capture_userptr(unsigned int buffer_size);
static error_t init_capture_mmap(void);
static error_t capture_read_frame(void);
static void capture_process_frame(const void *capture_frame);
static int xioctl(int fd, int request, void *arg);

/* Global functions */
/******************************************************************************/
/*                                                                            */
/* Function:    open_capture_device                                           */
/*                                                                            */
/* Description: Checks and opens the capture device.                          */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_STAT, if device node is not present.       */
/*                         - ERROR_ISCHR, if device isn't a character device. */
/*                         - ERROR_OPEN, if the open system call fails.       */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
error_t open_capture_device(void)
{
    struct stat st;
	error_t ret = SUCCESS;

    if (-1 == stat (conf_device_name_cap, &st)) {
		ret = ERROR_STAT;
		goto err0;
    }

    if (!S_ISCHR (st.st_mode)) {
		ret = ERROR_ISCHR;
		goto err0;
    }

    fd_cap = open(conf_device_name_cap, O_RDWR /* required */| O_NONBLOCK, 0);

    if (-1 == fd_cap) {
		ret = ERROR_OPEN;
		goto err0;
    }

	return SUCCESS;
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    close_capture_device                                          */
/*                                                                            */
/* Description: Closes the capture device.               		              */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_CLOSE, if the close system call fails.     */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
error_t close_capture_device(void)
{
	error_t ret = SUCCESS;
	if (close(fd_cap) == -1) {
		ret = ERROR_CLOSE;
		goto err0;
	}
	
	return SUCCESS;
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    init_capture_device                                           */
/*                                                                            */
/* Description: Queries the capture device capabilities, sets all cropping    */
/*              and format parameters and initializes the requested           */
/*              I/O buffer method if possible.                                */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_VIDIOC_QUERYCAP, if query device caps      */
/*                                                  fails.                    */
/*                         - ERROR_VIDIOC_CROPCAP, if query crop caps fails.  */
/*                         - ERROR_VIDIOC_S_CROP, if crop params could not    */
/*                                               be set.                      */
/*                         - ERROR_VIDIOC_S_FMT, if capture format could not  */
/*                                               be set.                      */
/*                         - ERROR_NOT_A_CAP_DEV, if current device is not a  */
/*                                                capture device.             */
/*                         - ERROR_VID_NOT_IO_READ, if I/O read method is not */
/*                                                  supported but requested.  */
/*                         - ERROR_VID_NOT_IO_STREAM, if I/O streaming method */
/*                                                    is not supported but    */
/*                                                    requested.              */
/*                         - ERROR_CALLOC, if cache allocationfor the         */
/*                                           buffers struct fails.            */
/*                         - ERROR_MALLOC, if memory allocation for the       */
/*                                           capture frambuffer fails.        */
/*                         - ERROR_VIDIOC_REQBUFS, if I/O buffer request      */
/*                                                 fails.                     */
/*                         - ERROR_VIDEOC_QUERYBUF, I/O buffer query fails.   */
/*                         - ERROR_MMAP, if memory mapping of the frame       */
/*                                        buffer fails.                       */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
error_t init_capture_device(void)
{
	error_t ret = SUCCESS;
	struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

	CLEAR(cap);
    if (0 > xioctl (fd_cap, VIDIOC_QUERYCAP, &cap)) {
		ret = ERROR_VIDIOC_QUERYCAP;
		goto err0;
    }

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		ret = ERROR_VID_NOT_A_CAP_DEV;
    }

    switch (conf_io_method) {
		case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
				ret = ERROR_VID_NOT_IO_READ;
            }
        break;
        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
				ret = ERROR_VID_NOT_IO_STREAM;
            }
        break;
		default:
		break;
	}

	if(ret)
		goto err0;
	
	/* Select video input, video standard and tune here. */
	CLEAR (cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl (fd_cap, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */
		if (0 > xioctl (fd_cap, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported, OK. */
                break;
                default:
                    /* Error */
					ret = ERROR_VIDIOC_S_CROP;
                break;
            }
        } else {
			/* No Error */
		}
    } else {        
        /* Error */
		ret = ERROR_VIDIOC_CROPCAP;
    }

	if(ret)
		goto err0;

	/* set up video format */
    CLEAR (fmt);
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = conf_frame_width;
    fmt.fmt.pix.height      = conf_frame_height;
    fmt.fmt.pix.pixelformat = conf_pixelformat;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY; /* TODO: adjustable */

    if (0 > xioctl (fd_cap, VIDIOC_S_FMT, &fmt)) {
        ret = ERROR_VIDIOC_S_FMT;
		goto err0;
	}
        
	/* NOTE: VIDIOC_S_FMT may change width and height. */
	/* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    switch (conf_io_method) {
		case IO_METHOD_READ:
			ret = init_capture_read(fmt.fmt.pix.sizeimage);
		break;
		case IO_METHOD_MMAP:
			ret = init_capture_mmap();
		break;
		case IO_METHOD_USERPTR:
            ret = init_capture_userptr(fmt.fmt.pix.sizeimage);
        break;
	}		

	if(ret)
		goto err0;

	return SUCCESS;
	
err0:	
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    uninit_capture_device                                         */
/*                                                                            */
/* Description: Releases all resources allocated by the I/O buffer methods.   */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_MUNMAP, if capture framebuffer memory      */
/*                                         could not be unmaped.              */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
error_t uninit_capture_device(void)
{
	error_t ret = SUCCESS;
	unsigned int i = 0;


    switch (conf_io_method) {
        case IO_METHOD_READ:
            free (capture_buffers[0].start);
        break;
        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i) {
				if (0 > munmap(capture_buffers[i].start, capture_buffers[i].length)) {
					ret = ERROR_MUNMAP;					
				}
			}
        break;
        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i) {
                free (capture_buffers[i].start);
			}
        break;
    }
    free (capture_buffers);
	
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    start_capturing                                               */
/*                                                                            */
/* Description: Starts the capturing process.                                 */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_VIDIOC_QBUF, if data exchange buffer       */
/*                                               cannot be queued.            */
/*                         - ERROR_VIDIOC_STREAMON, if the VIDIOC_STREAMON    */
/*                                                  IOCTL fails.              */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
error_t start_capturing(void)
{
	error_t ret = SUCCESS;
    unsigned int i;
    enum v4l2_buf_type type;

    switch (conf_io_method) {
        case IO_METHOD_READ:
            /* Nothing to do. */
        break;
        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;
				CLEAR (buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (0 > xioctl (fd_cap, VIDIOC_QBUF, &buf)) {
                    ret = ERROR_VIDIOC_QBUF;
					goto err0;
				}
            }
                
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (0 > xioctl (fd_cap, VIDIOC_STREAMON, &type)) {
				ret = ERROR_VIDIOC_STREAMON;
				goto err0;
			}
		break;
        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;
				CLEAR (buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;
                buf.index = i;
                buf.m.userptr = (unsigned long) capture_buffers[i].start;
                buf.length = capture_buffers[i].length;

                if (0 > xioctl (fd_cap, VIDIOC_QBUF, &buf)) {
                    ret = ERROR_VIDIOC_QBUF;
					goto err0;
                }
			}
                
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (0 > xioctl (fd_cap, VIDIOC_STREAMON, &type)) {
				ret = ERROR_VIDIOC_STREAMON;
				goto err0;
			}
	
        break;
    }

	return SUCCESS;
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    stop_capturing                                                */
/*                                                                            */
/* Description: Stops the capturing process.                                  */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_VIDIOC_STREAMON, if the VIDIOC_STREAMOFF   */
/*                                                  IOCTL fails.              */ 
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
error_t stop_capturing(void)
{
 	error_t ret = SUCCESS;
    enum v4l2_buf_type type;

    switch (conf_io_method) {
        case IO_METHOD_READ:
            /* Nothing to do. */
        break;
        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl (fd_cap, VIDIOC_STREAMOFF, &type)) {
                    ret = ERROR_VIDIOC_STREAMOFF;
					goto err0;
			}
        break;
    }
		
	return SUCCESS;
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    capture_mainloop                                              */
/*                                                                            */
/* Description: Main loop to exchange the video buffers.         			  */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_SELECT, if the select system call fails.   */
/*                         - ERROR_TIMEOUT, if the specified timeout is       */
/*                                          exceeded.                         */
/*                         - ERROR_READ, if the read system call fails.       */
/*                         - ERROR_VIDIOC_DQBUF, if data exchange buffer      */
/*                                               cannot be dequeued.          */
/*                         - ERROR_VIDIOC_QBUF, if data exchange buffer       */
/*                                               cannot be queued.            */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
error_t capture_mainloop(void) {
	error_t ret = SUCCESS;
	unsigned int count;
    fd_set fds;
    struct timeval tv;
    int r;

    count = conf_frame_count;
    while (count-- > 0) {
        for (;;) {
            FD_ZERO (&fds);
            FD_SET (fd_cap, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select (fd_cap + 1, &fds, NULL, NULL, &tv);
			if (-1 == r) {
                if (EINTR == errno)
                    continue;
				ret = ERROR_SELECT;
				goto err0;
            }

            if (0 == r) {
				ret = ERROR_TIMEOUT;
				goto err0;
			}

            ret = capture_read_frame ();
			
			if(SUCCESS == ret)
				break;
			else if(AGAIN == ret)
				;/* do nothing */
			else /* ERROR*/
				goto err0;
 
            /* EAGAIN - continue select loop. */
        }
    }
	return SUCCESS;
err0:
	return ret;
}

/* Local helper functions */
/******************************************************************************/
/*                                                                            */
/* Function:    init_capture_read                                             */
/*                                                                            */
/* Description: Initialises the capture device for I/O method 'read'.         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_CALLOC, if cache allocationfor the         */
/*                                           buffers struct fails.            */
/*                           ERROR_MALLOC, if memory allocation for the       */
/*                                           capture frambuffer fails.        */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
static error_t init_capture_read(unsigned int buffer_size)
{
	error_t ret = SUCCESS;

	capture_buffers = calloc(1, sizeof (*capture_buffers));

    if (!capture_buffers) {
		ret = ERROR_CALLOC;
		goto err0;
    }

    capture_buffers[0].length = buffer_size;
    capture_buffers[0].start = malloc(buffer_size);

    if (!capture_buffers[0].start) {
		ret = ERROR_MALLOC;
		goto err1;
    }

	return SUCCESS;
err1:
	free(capture_buffers);
err0:
	return ret;
}
/******************************************************************************/
/*                                                                            */
/* Function:    init_capture_userptr                                          */
/*                                                                            */
/* Description: Initialises the capture device for I/O method 'usrptr'.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_VIDIOC_REQBUFS, if I/O buffer request      */
/*                                                 fails.                     */
/*                         - ERROR_CALLOC, if cache allocationfor the         */
/*                                           buffers struct fails.            */
/*                           ERROR_MEMALIGN, if aligned memory allocation     */
/*                                           for the capture frambuffer       */
/*                                           fails.                           */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
static error_t init_capture_userptr(unsigned int buffer_size)
{
	error_t ret = SUCCESS;
    struct v4l2_requestbuffers req;
    unsigned int page_size;

    page_size = getpagesize ();
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);
	CLEAR (req);
	req.count = N_BUFFERS;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (0 > xioctl (fd_cap, VIDIOC_REQBUFS, &req)) {
 		ret = ERROR_VIDIOC_REQBUFS;
		goto err0;
    }

    capture_buffers = calloc (N_BUFFERS, sizeof (*capture_buffers));

    if (!capture_buffers) {
 		ret = ERROR_CALLOC;
		goto err0;
    }

    for (n_buffers = 0; n_buffers < N_BUFFERS; ++n_buffers) {
        capture_buffers[n_buffers].length = buffer_size;
        capture_buffers[n_buffers].start = memalign (/* boundary */ page_size,
                                                     buffer_size);

        if (!capture_buffers[n_buffers].start) {
			ret = ERROR_MEMALIGN;
			goto err1;
        }
    }

	return SUCCESS;
err1:
	free(capture_buffers);
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    init_capture_mmap                                             */
/*                                                                            */
/* Description: Initialises the capture device for I/O method 'mmap'.         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - ERROR_VIDIOC_REQBUFS, if I/O buffer request      */
/*                                                 fails.                     */
/*                         - ERROR_VIDEOC QUERYBUF, I/O buffer query fails.   */
/*                         - ERROR_CALLOC, if cache allocationfor the         */
/*                                           buffers struct fails.            */
/*                         - ERROR_MMAP, if memory mapping of the frame       */
/*                                        buffer fails.                       */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
static error_t init_capture_mmap(void)
{
	error_t ret = SUCCESS;
    struct v4l2_requestbuffers req;

   CLEAR (req);
    req.count = N_BUFFERS;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (0 > xioctl(fd_cap, VIDIOC_REQBUFS, &req)) {
		ret = ERROR_VIDIOC_REQBUFS;
		goto err0;
    }

    if (req.count < 2) {
		ret = ERROR_VID_NOT_ENOUGH_IO_BUF;
		goto err0;
    }

    capture_buffers = calloc (req.count, sizeof (*capture_buffers));

    if (!capture_buffers) {
		ret = ERROR_CALLOC;
		goto err0;
    }

   for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

		CLEAR (buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

       if (0 > xioctl (fd_cap, VIDIOC_QUERYBUF, &buf)) {
			ret = ERROR_VIDIOC_QUERYBUF;
			goto err1;
		}

        capture_buffers[n_buffers].length = buf.length;
        capture_buffers[n_buffers].start =
        mmap (NULL /* start anywhere */,
              buf.length,
              PROT_READ | PROT_WRITE /* required */,
              MAP_SHARED /* recommended */,
              fd_cap, buf.m.offset);

        if (MAP_FAILED == capture_buffers[n_buffers].start) {
			ret = ERROR_MMAP;
			goto err1;
		}
    }

	return SUCCESS;
	
err1:
	free(capture_buffers);
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    capture_read_frame                                            */
/*                                                                            */
/* Description: Reads one captured frame.                                     */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - AGAIN, if data not present.                      */
/*                         - ERROR_READ, if the read system call fails.       */
/*                         - ERROR_VIDIOC_DQBUF, if data exchange buffer      */
/*                                               cannot be dequeued.          */
/*                         - ERROR_VIDIOC_QBUF, if data exchange buffer       */
/*                                               cannot be queued.            */
/*              On success - SUCESS.                                          */
/*                                                                            */
/******************************************************************************/
static error_t capture_read_frame (void)
{
	error_t ret = SUCCESS;
    struct v4l2_buffer buf;
    unsigned int i;

    switch (conf_io_method) {
		case IO_METHOD_READ:
            if (-1 == read (fd_cap, capture_buffers[0].start, capture_buffers[0].length)) {
				switch (errno) {
					case EAGAIN:
						ret = AGAIN;
						goto err0;
					break;
					case EIO:
						/* Could ignore EIO, see spec. */
						/* fall through */
					break;
					default:
						ret = ERROR_READ;
						goto err0;
					break;
				}
            }

            capture_process_frame(capture_buffers[0].start);
		break;
        case IO_METHOD_MMAP:
            CLEAR (buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl (fd_cap, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        ret = AGAIN;
						goto err0;
					break;
                    case EIO:
                        /* Could ignore EIO, see spec. */
						/* fall through */
					break;
                    default:
                        ret = ERROR_VIDIOC_DQBUF;
						goto err0;
					break;
                }
            }

            assert (buf.index < n_buffers);

            capture_process_frame(capture_buffers[buf.index].start);

            if (-1 == xioctl (fd_cap, VIDIOC_QBUF, &buf)) {
                    ret = ERROR_VIDIOC_QBUF;
					goto err0;
			}
        break;
        case IO_METHOD_USERPTR:
            CLEAR (buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == xioctl (fd_cap, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        ret = AGAIN;
						goto err0;
					break;
                    case EIO:
                        /* Could ignore EIO, see spec. */
						/* fall through */
					break;
                    default:
                        ret = ERROR_VIDIOC_DQBUF;
						goto err0;
					break;
                }
			}

            for (i = 0; i < n_buffers; ++i)
                if (buf.m.userptr == (unsigned long) capture_buffers[i].start
                    && buf.length == capture_buffers[i].length)
                    break;

            assert (i < n_buffers);

            capture_process_frame((void *) buf.m.userptr);

            if (-1 == xioctl (fd_cap, VIDIOC_QBUF, &buf)) {
                        ret = ERROR_VIDIOC_QBUF;
						goto err0;
			}
        break;
    }

	return SUCCESS;
err0:
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* Function:    capture_process_frame                                         */
/*                                                                            */
/* Description: Processes a captured frame.                                   */
/*                                                                            */
/* Input:       capture_frame - The captured framebuffer.                     */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      None.                                                         */
/*                                                                            */
/******************************************************************************/
static void capture_process_frame(const void *capture_frame)
{
	switch(conf_pixelformat) {
		case V4L2_PIX_FMT_YUV420:
			format_yuv420_fb(capture_frame);
		break;
		default:
		/* currently unsupported pixelformats, do nothing */
		break;
	}
}

/******************************************************************************/
/*                                                                            */
/* Function:    xioctl                                             			  */
/*                                                                            */
/* Description: IOCTL for non blocking mode. 						          */
/*                                                                            */
/* Input:       fd      - File descriptor of the device.                      */
/*              request - IOCTL                                               */
/*              arg     - Parameter of the IOCTL                              */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      Return value from IOCTL.                                      */
/*                                                                            */
/******************************************************************************/
static int xioctl(int fd, int request, void *arg)
{
    int r;

    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}
