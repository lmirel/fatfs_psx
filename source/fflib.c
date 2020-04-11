//fflib.c

#include "types.h"
#include "storage.h"
#include <malloc.h>
#include <sys/file.h>
#include <lv2/mutex.h> 
#include <sys/errno.h>
#include <stdio.h>

#include "ff.h"
#include "fflib.h"

#define VER "v0.2.0"

//extern void NPrintf (const char* fmt, ...);
#define NPrintf(fmt, ...)

static u64 ffdev_id[MAXFDS];
static int ffdev_fd[MAXFDS];
static int ffdev_ss[MAXFDS];

static char _fflib_init = 0;

int fflib_init()
{
    int k;
    if (_fflib_init == 1)
        return FR_EXIST;
    for (k = 0; k < MAXFDS; k++)
    {
        ffdev_id[k] = 0;
        ffdev_fd[k] = -1;
        ffdev_ss[k] = -1;
    }
    _fflib_init = 1;
    return FR_OK;
}

int fflib_is_fatfs (char *path)
{
    FDIR dir;
    FATFS fs;       /* Work area (filesystem object) for logical drive */
    if (f_mount (&fs, path, 0) == FR_OK)
    {
        if (f_opendir (&dir, path) == FR_OK)
        {
            f_closedir (&dir);
            f_mount (0, path, 0);
            return FR_OK;
        }
        f_mount (0, path, 0);
    }
    //
    return FR_NO_FILESYSTEM;
}

int fflib_attach(int idx, u64 id, int now)
{
    if (idx < 0 || idx > MAXFDS)
    {
        NPrintf ("!fflib_attach: invalid drive index %d for 0x%llx %d\n", idx, id, now);
        return FR_INVALID_DRIVE;
    }
    if (ffdev_id[idx] > 0)
    {
        NPrintf ("!fflib_attach: drive already attached as 0x%llx on index %d for 0x%llx %d\n", ffdev_id[idx], idx, id, now);
        return FR_EXIST;
    }
    fflib_init ();
    //
    ffdev_id[idx] = id;
    //
    if (now)
    {
        //mount fs and check access
        FDIR dir;
        FATFS fs;       /* Work area (filesystem object) for logical drive */
        char drv[7];    //MAXFDS MUST not be more than 3 digits
        snprintf (drv, 7, "%d:/", idx);
        if(f_mount (&fs, drv, 0) != FR_OK)
        {
            NPrintf ("!fflib_attach: mount failed for 0x%llx on index %d\n", ffdev_id[idx], idx);
            return FR_NOT_ENABLED;
        }
        /* Open the directory */
        int res = f_opendir (&dir, drv);
        if (res != FR_OK)
        {
            f_mount (0, drv, 0);
            NPrintf ("!fflib_attach: can't open drive for 0x%llx on index %d res %d\n", ffdev_id[idx], idx, res);
            return FR_NO_FILESYSTEM;
        }
        f_closedir (&dir);
        f_mount (0, drv, 0);
    }
    return FR_OK;
}

int fflib_detach(int idx)
{
    if (idx < 0 || idx > MAXFDS)
    {
        NPrintf ("!fflib_detach: invalid drive index %d for 0x%llx %d\n", idx);
        return FR_INVALID_DRIVE;
    }
    fflib_init ();
    if(ffdev_id[idx] < 0)
    {
        NPrintf ("!fflib_detach: can't detach 0x%llx on index %d\n", ffdev_id[idx], idx);
        return FR_NOT_READY;
    }
    if(ffdev_fd[idx] > 0)
        sys_storage_close (ffdev_fd[idx]);
    ffdev_id[idx] = 0;
    ffdev_fd[idx] = -1;
    ffdev_ss[idx] = -1;

    return FR_OK;
}

u64 fflib_id_get(int idx)
{
    fflib_init ();
    return ffdev_id[idx];
}

int fflib_fd_get(int idx)
{
    fflib_init ();
    return ffdev_fd[idx];
}

int fflib_ss_get(int idx)
{
    fflib_init ();
    return ffdev_ss[idx];
}

int fflib_fd_set(int idx, int fd)
{
    fflib_init ();
    ffdev_fd[idx] = fd;

    return fd;
}

int fflib_ss_set(int idx, int ss)
{
    fflib_init ();
    ffdev_ss[idx] = ss;

    return ss;
}

int fflib_file_to_sectors(const char *path, uint32_t *sec_out, uint32_t *size_out, int max, int phys)
{
#if 0
    int is_ntfs = 0; 
    if(!strncmp(path, "ntfs", 4) || !strncmp(path, "/ntfs", 5) ||
       !strncmp(path, "ext", 3) || !strncmp(path, "/ext", 4)) is_ntfs = 1;

    reent1._errno = 0;

    ps3ntfs_init();

    if(is_ntfs) {

        if(path[0]=='/') path++;

    }

    else {
        return -1;
    }
    
    return devoptab_list[get_dev2(path)]->file_to_sectors(&reent1, (void *) path, sec_out, size_out, max, phys);
#endif
    return FR_OK;
}
