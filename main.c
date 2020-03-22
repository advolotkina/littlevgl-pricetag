#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

#define DISP_BUF_SIZE (80*LV_HOR_RES_MAX)

#define CPU_LABEL_COLOR     "FF0000"
#define MEM_LABEL_COLOR     "0000FF"

static lv_obj_t * win;
static lv_obj_t * name_label;
static lv_obj_t * price_label;
static lv_obj_t * good_pic;
// static lv_obj_t * specs_list;
#define PC_FILES    1 
typedef  FILE * pc_file_t;
static lv_fs_res_t pcfs_open(lv_fs_drv_t * drv, void * file_p, const char * fn, lv_fs_mode_t mode);
static lv_fs_res_t pcfs_close(lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t pcfs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t pcfs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos);
static lv_fs_res_t pcfs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);

int main(int argc, char** argv)
{
    char buf_long[256];
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

    win = lv_win_create(lv_disp_get_scr_act(NULL), NULL);
    lv_win_set_title(win, "Smart price tag");    
    lv_win_set_layout(win, LV_LAYOUT_PRETTY);

    name_label = lv_label_create(win, NULL);
    lv_label_set_recolor(name_label, true);
    lv_obj_align(name_label, NULL, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

    price_label = lv_label_create(win, NULL);
    lv_label_set_recolor(name_label, true);
    lv_obj_align(name_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    sprintf(buf_long, "%s", argv[1]);
    lv_label_set_text(name_label, buf_long);
    sprintf(buf_long, "%s $$", argv[2]);
    lv_label_set_text(price_label, buf_long);

    good_pic = lv_img_create(win, NULL);
    lv_img_set_src(good_pic, "P/opt/goods/good1/good.bin");
    lv_obj_set_pos(good_pic, 150, 200);      /*Set the positions*/
    lv_obj_set_drag(good_pic, true);
    // /*Create a scroll bar style*/
    // static lv_style_t style_sb;
    // lv_style_copy(&style_sb, &lv_style_plain);
    // style_sb.body.main_color = LV_COLOR_BLACK;
    // style_sb.body.grad_color = LV_COLOR_BLACK;
    // style_sb.body.border.color = LV_COLOR_WHITE;
    // style_sb.body.border.width = 1;
    // style_sb.body.border.opa = LV_OPA_70;
    // style_sb.body.radius = LV_RADIUS_CIRCLE;
    // style_sb.body.opa = LV_OPA_60;
    // style_sb.body.padding.right = 3;
    // style_sb.body.padding.bottom = 3;
    // style_sb.body.padding.inner = 8;        /*Scrollbar width*/

    // specs_list =  lv_page_create(win, NULL);
    // lv_obj_set_size(specs_list, 150, 200);
    // lv_obj_align(specs_list, NULL, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);
    // lv_page_set_style(specs_list, LV_PAGE_STYLE_SB, &style_sb);           /*Set the scrollbar style*/

    // /*Create a label on the page*/
    // lv_obj_t * label = lv_label_create(specs_list, NULL);
    // lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);            /*Automatically break long lines*/
    // lv_obj_set_width(label, lv_page_get_fit_width(specs_list));          /*Set the label width to max value to not show hor. scroll bars*/
    // lv_label_set_text(label, "Lorem ipsum dolor sit amet, consectetur adipiscing elit,\n"
    //                          "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    //                          "Ut enim ad minim veniam, quis nostrud exercitation ullamco\n"
    //                          "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure\n"
    //                          "dolor in reprehenderit in voluptate velit esse cillum dolore\n"
    //                          "eu fugiat nulla pariatur.\n"
    //                          "Excepteur sint occaecat cupidatat non proident, sunt in culpa\n"
    //                          "qui officia deserunt mollit anim id est laborum.");

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
