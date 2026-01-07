#include "lv_fs_spiffs.h"

#include <stdio.h>
#include <string.h>
#include "lvgl.h"

static const char *LV_FS_SPIFFS_BASE = "/spiffs";

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    (void)drv;
    if (mode != LV_FS_MODE_RD) {
        return NULL;
    }
    if (!path) {
        return NULL;
    }
    if (path[0] == '/') {
        path++;
    }
    char full_path[256];
    int written = snprintf(full_path, sizeof(full_path), "%s/%s", LV_FS_SPIFFS_BASE, path);
    if (written <= 0 || (size_t)written >= sizeof(full_path)) {
        return NULL;
    }
    return fopen(full_path, "rb");
}

static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p)
{
    (void)drv;
    FILE *fp = (FILE *)file_p;
    if (!fp) {
        return LV_FS_RES_INV_PARAM;
    }
    fclose(fp);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr,
                           uint32_t *br)
{
    (void)drv;
    if (!file_p || !buf || !br) {
        return LV_FS_RES_INV_PARAM;
    }
    *br = (uint32_t)fread(buf, 1, btr, (FILE *)file_p);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos,
                           lv_fs_whence_t whence)
{
    (void)drv;
    if (!file_p) {
        return LV_FS_RES_INV_PARAM;
    }
    int origin = SEEK_SET;
    if (whence == LV_FS_SEEK_CUR) {
        origin = SEEK_CUR;
    } else if (whence == LV_FS_SEEK_END) {
        origin = SEEK_END;
    }
    if (fseek((FILE *)file_p, (long)pos, origin) != 0) {
        return LV_FS_RES_FS_ERR;
    }
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    (void)drv;
    if (!file_p || !pos_p) {
        return LV_FS_RES_INV_PARAM;
    }
    long pos = ftell((FILE *)file_p);
    if (pos < 0) {
        return LV_FS_RES_FS_ERR;
    }
    *pos_p = (uint32_t)pos;
    return LV_FS_RES_OK;
}

void lv_fs_spiffs_init(void)
{
    lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);
    drv.letter = 's';
    drv.open_cb = fs_open;
    drv.close_cb = fs_close;
    drv.read_cb = fs_read;
    drv.seek_cb = fs_seek;
    drv.tell_cb = fs_tell;
    lv_fs_drv_register(&drv);
}
