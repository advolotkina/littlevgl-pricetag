#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DISP_BUF_SIZE (80*LV_HOR_RES_MAX)

typedef  FILE * pc_file_t;
static lv_fs_res_t pcfs_open(lv_fs_drv_t * drv, void * file_p, const char * fn, lv_fs_mode_t mode);
static lv_fs_res_t pcfs_close(lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t pcfs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t pcfs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos);
static lv_fs_res_t pcfs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);


int main(int argc, char** argv)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    evdev_init();
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*See below.*/
    indev_drv.read_cb = evdev_read;            /*See below.*/
    lv_indev_drv_register(&indev_drv);      /*Register the driver in LittlevGL*/

    /* Add a simple drive to open images from PC*/
    lv_fs_drv_t pcfs_drv;                         /*A driver descriptor*/
    memset(&pcfs_drv, 0, sizeof(lv_fs_drv_t));    /*Initialization*/

    pcfs_drv.file_size = sizeof(pc_file_t);       /*Set up fields...*/
    pcfs_drv.letter = 'P';
    pcfs_drv.open_cb = pcfs_open;
    pcfs_drv.close_cb = pcfs_close;
    pcfs_drv.read_cb = pcfs_read;
    pcfs_drv.seek_cb = pcfs_seek;
    pcfs_drv.tell_cb = pcfs_tell;
    lv_fs_drv_register(&pcfs_drv);


    lv_theme_t * th = lv_theme_night_init(210, &jost_28);     //Set a HUE value and a Font for the Night Theme
    lv_theme_set_current(th);   

    // Считать количество товаров из файла
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen ( "/opt/goods/count" , "rb" );
    if( !fp ) perror("/opt/goods/count"),exit(1);

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
      fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

    int count = atoi(buffer);

    fclose(fp);
    free(buffer);

    lv_obj_t *tab[count];
    lv_obj_t *page[count];
    lv_obj_t *name_label[count];
    lv_obj_t *price_label[count];
    lv_obj_t *specs[count];
    lv_obj_t *good_pic[count];


    int size = 100;
    char *path;
    char *buf_long;
    path = malloc(size);

    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), NULL);

    static lv_style_t style;
    lv_style_copy(&style, &lv_style_plain);
    style.text.color = LV_COLOR_WHITE;
    style.text.font = &jost_bold_40;

    for (int i = 0; i < count; i++){
        tab[i] = lv_tabview_add_tab(tabview, "Prod");
        lv_obj_align(tab[i], NULL, LV_ALIGN_CENTER, 0, 0);
        
        good_pic[i] = lv_img_create(tab[i], NULL);
        memset(path, 0, size);
        sprintf(path, "P/opt/goods/good%d/good.bin", i + 1);
        lv_img_set_src(good_pic[i], path);
        
        page[i] = lv_page_create(tab[i], NULL);
        lv_obj_set_size(page[i], 370, 370);
        lv_obj_align(page[i], NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0); 

        name_label[i] = lv_label_create(page[i], NULL);
        lv_label_set_style(name_label[i], LV_LABEL_STYLE_MAIN, &style);
        lv_label_set_long_mode(name_label[i], LV_LABEL_LONG_BREAK);
        lv_obj_set_width(name_label[i], 340); 

        price_label[i] = lv_label_create(page[i], NULL);
        lv_label_set_long_mode(price_label[i], LV_LABEL_LONG_BREAK);
        lv_obj_set_width(price_label[i], 340); 
        lv_obj_align(price_label[i], NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

        specs[i] = lv_label_create(tab[i], NULL);
        lv_label_set_long_mode(specs[i], LV_LABEL_LONG_BREAK); 
        lv_label_set_recolor(specs[i], true);
        lv_obj_set_width(specs[i], 740);
        lv_obj_align(specs[i], NULL, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_pos(specs[i], 0, 410);

        memset(path, 0, size);
        sprintf(path, "/opt/goods/good%d/name", i + 1);

        fp = fopen ( path , "rb" );
        if( !fp ) perror(path),exit(1);

        fseek( fp , 0L , SEEK_END);
        lSize = ftell( fp );
        rewind( fp );

        /* allocate memory for entire content */
        buffer = calloc( 1, lSize+1 );
        if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

        /* copy the file into the buffer */
        if( 1!=fread( buffer , lSize, 1 , fp) )
          fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

        lv_label_set_text(name_label[i], buffer);

        fclose(fp);
        free(buffer);

        memset(path, 0, size);
        sprintf(path, "/opt/goods/good%d/price", i + 1);

        fp = fopen ( path , "rb" );
        if( !fp ) perror(path),exit(1);

        fseek( fp , 0L , SEEK_END);
        lSize = ftell( fp );
        rewind( fp );

        /* allocate memory for entire content */
        buffer = calloc( 1, lSize+1 );
        if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

        /* copy the file into the buffer */
        if( 1!=fread( buffer , lSize, 1 , fp) )
          fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

        buf_long = calloc( 1, lSize + 11);
        sprintf(buf_long, "Цена: %s", buffer);
        lv_label_set_text(price_label[i], buf_long);

        fclose(fp);
        free(buffer);
        free(buf_long);

        memset(path, 0, size);
        sprintf(path, "/opt/goods/good%d/specs", i + 1);

        fp = fopen ( path , "rb" );
        if( !fp ) perror(path),exit(1);

        fseek( fp , 0L , SEEK_END);
        lSize = ftell( fp );
        rewind( fp );

        /* allocate memory for entire content */
        buffer = calloc( 1, lSize+1 );
        if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

        /* copy the file into the buffer */
        if( 1!=fread( buffer , lSize, 1 , fp) )
          fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

        lv_label_set_text(specs[i], buffer);

        fclose(fp);
        free(buffer);
    }
    free(path);


    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}


/**
 * Open a file from the PC
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE* variable
 * @param fn name of the file.
 * @param mode element of 'fs_mode_t' enum or its 'OR' connection (e.g. FS_MODE_WR | FS_MODE_RD)
 * @return LV_FS_RES_OK: no error, the file is opened
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t pcfs_open(lv_fs_drv_t * drv, void * file_p, const char * fn, lv_fs_mode_t mode)
{
    (void) drv; /*Unused*/

    errno = 0;

    const char * flags = "";

    if(mode == LV_FS_MODE_WR) flags = "wb";
    else if(mode == LV_FS_MODE_RD) flags = "rb";
    else if(mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) flags = "a+";

    /*Make the path relative to the current directory (the projects root folder)*/
    char buf[256];

    sprintf(buf, "/%s", fn);

    pc_file_t f = fopen(buf, flags);
    if(f == NULL) return LV_FS_RES_UNKNOWN;
    else {
        fseek(f, 0, SEEK_SET);

        /* 'file_p' is pointer to a file descriptor and
         * we need to store our file descriptor here*/
        pc_file_t * fp = file_p;        /*Just avoid the confusing casings*/
        *fp = f;
    }

    return LV_FS_RES_OK;
}


/**
 * Close an opened file
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE* variable. (opened with lv_ufs_open)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv__fs_res_t enum
 */
static lv_fs_res_t pcfs_close(lv_fs_drv_t * drv, void * file_p)
{
    (void) drv; /*Unused*/

    pc_file_t * fp = file_p;        /*Just avoid the confusing casings*/
    fclose(*fp);
    return LV_FS_RES_OK;
}

/**
 * Read data from an opened file
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv__fs_res_t enum
 */
static lv_fs_res_t pcfs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    (void) drv; /*Unused*/

    pc_file_t * fp = file_p;        /*Just avoid the confusing casings*/
    *br = (uint32_t)fread(buf, 1, btr, *fp);
    return LV_FS_RES_OK;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE* variable. (opened with lv_ufs_open )
 * @param pos the new position of read write pointer
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv__fs_res_t enum
 */
static lv_fs_res_t pcfs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos)
{
    (void) drv; /*Unused*/

    pc_file_t * fp = file_p;        /*Just avoid the confusing casings*/
    fseek(*fp, pos, SEEK_SET);
    return LV_FS_RES_OK;
}

/**
 * Give the position of the read write pointer
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE* variable.
 * @param pos_p pointer to to store the result
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv__fs_res_t enum
 */
static lv_fs_res_t pcfs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    (void) drv; /*Unused*/
    pc_file_t * fp = file_p;        /*Just avoid the confusing casings*/
    *pos_p = ftell(*fp);
    return LV_FS_RES_OK;
}


/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
