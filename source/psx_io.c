//psx_io.c

#include "types.h"
#include "storage.h"
#include <malloc.h>
#include <sys/file.h>
#include <lv2/mutex.h> 
#include <sys/errno.h>
#include <stdio.h>

#include "ff.h"
#include "psx_io.h"

static u64 ffdev_id[MAXFDS];
static int ffdev_fd[MAXFDS];
static int ffdev_ss[MAXFDS];

int fflib_init()
{
    int k;
    for (k = 0; k < MAXFDS; k++)
    {
        ffdev_id[k] = 0;
        ffdev_fd[k] = -1;
        ffdev_ss[k] = -1;
    }
    return FR_OK;
}

int fflib_attach(int idx, u64 id, int now)
{
    if (idx < 0 || idx > MAXFDS)
        return FR_INVALID_DRIVE;
    if (ffdev_id[idx] > 0)
        return FR_EXIST;
    //
    ffdev_id[idx] = id;
    //
    if (now)
    {
        //mount fs and check access
        FDIR dir;
        FATFS fs;       /* Work area (filesystem object) for logical drive */
        char drv[7];    //MAXFDS MUST not be more than 3 digits
        snprintf(drv, 7, "%d:/", idx);
        if(f_mount(&fs, drv, 0) != FR_OK)
            return FR_NOT_ENABLED;
        /* Open the directory */
        if (f_opendir(&dir, drv) != FR_OK)
        {
            f_mount(0, drv, 0);
            return FR_NO_FILESYSTEM;
        }
        f_closedir(&dir);
        f_mount(0, drv, 0);
    }
    return FR_OK;
}

int fflib_detach(int idx, u64 id)
{
    if (idx < 0 || idx > MAXFDS)
        return FR_INVALID_DRIVE;

    if(ffdev_fd[idx] < 0)
        return FR_NOT_READY;

    sys_storage_close (ffdev_fd[idx]);
    ffdev_id[idx] = 0;
    ffdev_fd[idx] = -1;
    ffdev_ss[idx] = -1;

    return FR_OK;
}

u64 fflib_id_get(int idx)
{
    return ffdev_id[idx];
}

int fflib_fd_get(int idx)
{
    return ffdev_fd[idx];
}

int fflib_ss_get(int idx)
{
    return ffdev_ss[idx];
}

int fflib_fd_set(int idx, int fd)
{
    ffdev_fd[idx] = fd;

    return fd;
}

int fflib_ss_set(int idx, int ss)
{
    ffdev_ss[idx] = ss;

    return ss;
}
