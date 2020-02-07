/* 
   TINY3D sample / (c) 2010 Hermes  <www.elotrolado.net>

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include <io/pad.h>

#include <tiny3d.h>
#include <libfont.h>

// font 0: 224 chr from 32 to 255, 16 x 32 pix 2 bit depth
#include "font.h"

// font 1: 224 chr from 32 to 255, 16 x 32 pix 2 bit depth
#include "font_b.h"

// font 2: 255 chr from 0 to 254, 8 x 8 pix 1 bit depth
extern unsigned char msx[];

#include "ff.h"

#define SDTEST
#ifdef SDTEST
#include "types.h"

typedef struct {
    int device;
    void *dirStruct;
} DIR_ITER;

#include "iosupport.h"
#include "storage.h"
#include <malloc.h>
#include <sys/file.h>
#include <lv2/mutex.h> 
#include <sys/errno.h>

#include <sys/file.h>
#include <ppu-lv2.h>
#include <sys/stat.h>
#include <lv2/sysfs.h>

#include <sysutil/disc.h>

#include <sysmodule/sysmodule.h>

int fddr[8] = {0};
char fdld[8][256];
static u64 ff_ps3id[8] = {
	0x010300000000000AULL, 0x010300000000000BULL, 0x010300000000000CULL, 0x010300000000000DULL,
	0x010300000000000EULL, 0x010300000000000FULL, 0x010300000000001FULL, 0x0103000000000020ULL 
	};
#if 0
static int dev_fd[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
#include "storage.h"
int sdopen (int fd)
{
	static device_info_t disc_info;
	disc_info.unknown03 = 0x12345678; // hack for Iris Manager Disc Less
	disc_info.sector_size = 0;
	//int ret = sys_storage_get_device_info(ff_ps3id[fd], &disc_info);
    int ret = sys_storage_open(ff_ps3id[fd], &dev_fd[fd]);
    if (0 == ret)
        sys_storage_close(dev_fd[fd]);
    return ret;
}
#else
FRESULT scan_files2 (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    FDIR dir;
    UINT i = 0;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) 
    {
        for (;;) 
        {
            FRESULT res1 = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res1 != FR_OK || fno.fname[0] == 0) 
                break;  /* Break on error or end of dir */
            i++;
            if (i > 7)
                break;
            if (fno.fattrib & AM_DIR) 
            {                    /* It is a directory */
                snprintf (fdld[i], 255, "/%s", fno.fname);
                //sprintf(&path[i], "/%s", fno.fname);
                //res = scan_files(path);                    /* Enter the directory */
                //if (res != FR_OK) break;
                //path[i] = 0;
            } 
            else 
            {                                       /* It is a file. */
                snprintf (fdld[i], 255, "%s", fno.fname);
                //printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }
    return res;
}

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    FDIR dir;
    UINT i = 0;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK)
    {
        for (i = 0; i < 8; i++)
        {
            FRESULT res1 = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res1 != FR_OK || fno.fname[0] == 0) 
            {
                //res1 = disk_status(dir.obj.fs->pdrv);
                snprintf (fdld[i], 255, "%x !f_readdir %s drive %d ssize %d", res1, path, dir.obj.fs->pdrv, 0/*dir.obj.fs->ssize*/);
                //
                f_closedir(&dir);
                return res;  /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR) 
            {                    /* It is a directory */
                snprintf (fdld[i], 255, "/%s", fno.fname);
                //sprintf(&path[i], "/%s", fno.fname);
                //res = scan_files(path);                    /* Enter the directory */
                //if (res != FR_OK) break;
                //path[i] = 0;
            } 
            else 
            {                                       /* It is a file. */
                snprintf (fdld[i], 255, "%s", fno.fname);
                //printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }
    return res;
}

int sdopen (int i)
{
    //FDIR fdir;
    char lbuf[10];
    FATFS *fs;     /* Ponter to the filesystem object */
    fs = malloc(sizeof (FATFS));           /* Get work area for the volume */
    snprintf(lbuf, 10, "%d:/", i);
    f_mount(fs, lbuf, 0);                    /* Mount the default drive */
    int ret = scan_files(lbuf); //f_opendir (&fdir, lbuf);
    //if (ret == FR_OK)
    //    f_closedir (&fdir);
    f_mount(0, lbuf, 0);                    /* Mount the default drive */
    free(fs);
    //
    return ret;
}

int sdopen2 (int i)
{
    FDIR fdir;
    char lbuf[10];
    FATFS *fs;     /* Ponter to the filesystem object */
    fs = malloc(sizeof (FATFS));           /* Get work area for the volume */
    snprintf(lbuf, 10, "%d:/", i);
    int ret = f_mount(fs, lbuf, 0);                    /* Mount the default drive */
    if (ret != FR_OK)
    {
        free(fs);
        return ret;
    }
    ret = f_opendir (&fdir, lbuf);
    snprintf (fdld[i], 255, "%d f_opendir %s drive %d", ret, lbuf, i/*dir.obj.fs->ssize*/);
    if (ret == FR_OK)
        f_closedir (&fdir);
    f_mount(0, lbuf, 0);                    /* Mount the default drive */
    free(fs);
    //
    return ret;
}
#endif
//

int fatfs_init()
{
    //
    int k; for (k = 0; k < 8; k++)
    {
        //snprintf(lbuf, 10, "%d:/", i);
        //f_mount(fs, lbuf, 0);                    /* Mount the default drive */
        fdld[k][0] = '\0';
        fddr[k] = sdopen(k);
        //fddr[i] = f_opendir (&fdir, lbuf);
        //if (fddr[i] == FR_OK)
        //    f_closedir (&fdir);
        //f_mount(0, lbuf, 0);                    /* Mount the default drive */
    }
    return 0;
}
#endif
// draw one background color in virtual 2D coordinates

void DrawBackground2D(u32 rgba)
{
    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(0  , 0  , 65535);
    tiny3d_VertexColor(rgba);

    tiny3d_VertexPos(847, 0  , 65535);

    tiny3d_VertexPos(847, 511, 65535);

    tiny3d_VertexPos(0  , 511, 65535);
    tiny3d_End();
}

void drawScene()
{
	float x, y;
    char lbuf[256];

    tiny3d_Project2D(); // change to 2D context (remember you it works with 848 x 512 as virtual coordinates)
    //DrawBackground2D(0x0040ffff) ; // light blue 
    DrawBackground2D(0x000000ff) ; // light blue 

    SetFontSize(8, 8);
    
    x= 0.0; y = 0.0;

    SetCurrentFont(2);
    SetFontColor(0xffffffff, 0x0);
    int i;
    for (i = 0; i < 8; i++)
    {
        snprintf(lbuf, 255, "drive %d open result %d for 0x%llx", i, fddr[i], (long long unsigned int)ff_ps3id[i]);
        DrawString(x,y, lbuf);
        y += 8;
    }
    for (i = 0; i < 8; i++)
    {
        DrawString(x,y, fdld[i]);
        y += 8;
    }
    #if 0
    SetFontColor(0x00ff00ff, 0x0);
    SetCurrentFont(0);
    x = DrawString(x,y, "Hermes ");
    SetCurrentFont(1);
    SetFontColor(0xffffffff, 0x0);
    x = DrawString(x,y, "and this is one sample working with\nfonts.");

    SetCurrentFont(2);
    
    x= 0; y += 64;
    SetCurrentFont(1);
    DrawString(x, y, "I am using 3 fonts:");
    
    SetCurrentFont(0);
    y += 64;
    SetFontColor(0xffffffff, 0x00a000ff);
    DrawString(x, y, "Font 0 is one array of 224 chars 16 x 32 pix and 2 bit depth");

    SetCurrentFont(1);
    y += 64;
    SetFontColor(0xffffffff, 0xa00000ff);
    DrawString(x, y, "Font 1 is one array of 224 chars 16 x 32 pix and 2 bit depth");

    SetCurrentFont(2);
    y += 64;
    SetFontColor(0x000000ff, 0xffff00ff);
    DrawString(x, y, "Font 2 is one array of 255 chars 8 x 8 pix and 1 bit depth");

    y += 64;
    SetCurrentFont(1);
    SetFontSize(32, 64);
    SetFontColor(0xffffffff, 0x000000ff);
    SetFontAutoCenter(1);
    DrawString(0, y, "You can resize letters");
    SetFontAutoCenter(0);

    SetFontSize(12, 24);
    SetFontColor(0xffffffff, 0x00000000);
    y += 72;
    DrawString(0, y, "change the color, background color and center the text\nwith SetFontAutoCenter()");
    y += 72;

    SetFontColor(0x00ff00ff, 0x00000000);
    DrawFormatString(0, y, "Here %s font 0 uses %i bytes as texture", "using DrawFormatString()", 224*(16*2/8)*32);
    #endif
}


void LoadTexture()
{

    u32 * texture_mem = tiny3d_AllocTexture(64*1024*1024); // alloc 64MB of space for textures (this pointer can be global)    

    u32 * texture_pointer; // use to asign texture space without changes texture_mem

    if(!texture_mem) return; // fail!

    texture_pointer = texture_mem;

    ResetFont();
    texture_pointer = (u32 *) AddFontFromBitmapArray((u8 *) font  , (u8 *) texture_pointer, 32, 255, 16, 32, 2, BIT0_FIRST_PIXEL);
    texture_pointer = (u32 *) AddFontFromBitmapArray((u8 *) font_b, (u8 *) texture_pointer, 32, 255, 16, 32, 2, BIT0_FIRST_PIXEL);
    texture_pointer = (u32 *) AddFontFromBitmapArray((u8 *) msx   , (u8 *) texture_pointer,  0, 254,  8,  8, 1, BIT7_FIRST_PIXEL);

    // here you can add more textures using 'texture_pointer'. It is returned aligned to 16 bytes
}
//1st
int app_init (int dat)
{
    fatfs_init();

    tiny3d_Init(1024*1024);

	ioPadInit(7);

	// Load texture

    LoadTexture();
    int i;
    for (i = 0; i < 8; i++)
    {
        fddr[i] = sdopen(i);
    }
    //
    return 1;
}
//2nd
int app_input(int dat)
{
	padInfo padinfo;
	padData paddata;
    int i;
    // Check the pads.
    ioPadGetInfo(&padinfo);

    for(i = 0; i < MAX_PADS; i++){

        if(padinfo.status[i]){
            ioPadGetData(i, &paddata);
            
            if(paddata.BTN_CROSS){
                return 0;
            }
        }
        
    }
    return 1;
}
//3rd
int app_update(int dat)
{
    return 1;
}
//4th
int app_render(int dat)
{
    /* DRAWING STARTS HERE */

    // clear the screen, buffer Z and initializes environment to 2D

    tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

    // Enable alpha Test
    tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

    // Enable alpha blending.
    tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
        TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
        TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

    drawScene(); // Draw

    /* DRAWING FINISH HERE */

    tiny3d_Flip();

    return 1;
}
//5th
int app_cleanup(int dat)
{
    return 1;
}

s32 main(s32 argc, const char* argv[])
{
    //1 init
	app_init(0);
	// Ok, everything is setup. Now for the main loop.
	while(1) 
    {
        //2 input
        if (!app_input(0))
            return 0;
        //3
        app_update(0);
		//4
        app_render(0);
	}
    //5
    app_cleanup(0);
	return 0;
}

