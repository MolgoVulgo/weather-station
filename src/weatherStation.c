
// #include <Arduino.h>
#include <lvgl.h>
#include "display.h"
#include "esp_bsp.h"
#include "lv_port.h"
#include <esp_log.h>   // Add this line to include the header file that declares ESP_LOGI
#include <esp_flash.h> // Add this line to include the header file that declares esp_flash_t
#include <esp_chip_info.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "wifi_manager.h"
#include <driver/sdmmc_host.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <dirent.h>
#include "ui_screen.h"
#include "time_sync.h"

static const char *TAG = "WeatherStation";

#define logSection(section) \
  ESP_LOGI(TAG, "\n\n************* %s **************\n", section);

/**
 * @brief LVGL porting app
 * Set the rotation degree:
 *      - 0: 0 degree
 *      - 90: 90 degree
 *      - 180: 180 degree
 *      - 270: 270 degree
 *
 */
#define LVGL_PORT_ROTATION_DEGREE (90)

static esp_err_t sdcard_mount(void)
{
  sdmmc_card_t *card = NULL;
  const char *mount_point = "/sdcard";

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.width = 1;
  slot_config.clk = GPIO_NUM_12;
  slot_config.cmd = GPIO_NUM_11;
  slot_config.d0 = GPIO_NUM_13;

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
  };

  esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SD mount failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG, "SD mounted at %s", mount_point);
  sdmmc_card_print_info(stdout, card);
  return ESP_OK;
}

static void sdcard_list_dir(const char *path)
{
  DIR *dir = opendir(path);
  if (!dir) {
    ESP_LOGE(TAG, "SD opendir failed: %s", path);
    return;
  }

  ESP_LOGI(TAG, "SD contents of %s:", path);
  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL) {
    ESP_LOGI(TAG, "  %s", ent->d_name);
  }
  closedir(dir);
}

/**
 * To use the built-in demos of LVGL uncomment the include below.
 * You also need to copy `lvgl/demos` to `lvgl/src/demos`.
 */
// #include <demos/lv_demos.h>
#include "ui_backend.h"

void setup();

#if !CONFIG_AUTOSTART_ARDUINO
void app_main()
{
  // initialize arduino library before we start the tasks
  // initArduino();

  setup();
}
#endif
void setup()
{
  //  String title = "WeatherStation";

  vTaskDelay(pdMS_TO_TICKS(10000));
  logSection("WeatherStation start");
  esp_chip_info_t chip_info;
  uint32_t flash_size;
  esp_chip_info(&chip_info);
  ESP_LOGI(TAG, "This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

  unsigned major_rev = chip_info.revision / 100;
  unsigned minor_rev = chip_info.revision % 100;
  ESP_LOGI(TAG, "silicon revision v%d.%d, ", major_rev, minor_rev);
  if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
  {
    ESP_LOGI(TAG, "Get flash size failed");
    return;
  }

  ESP_LOGI(TAG, "%" PRIu32 "MB %s flash", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes", esp_get_minimum_free_heap_size());
  size_t freePsram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  ESP_LOGI(TAG, "Free PSRAM: %d bytes", freePsram);
  logSection("Initialize panel device");
  // ESP_LOGI(TAG, "Initialize panel device");
  bsp_display_cfg_t cfg = {
      .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
      .buffer_size = LCD_QSPI_H_RES * LCD_QSPI_V_RES,
#if LVGL_PORT_ROTATION_DEGREE == 90
      .rotate = LV_DISP_ROT_90,
#elif LVGL_PORT_ROTATION_DEGREE == 270
      .rotate = LV_DISP_ROT_270,
#elif LVGL_PORT_ROTATION_DEGREE == 180
      .rotate = LV_DISP_ROT_180,
#elif LVGL_PORT_ROTATION_DEGREE == 0
      .rotate = LV_DISP_ROT_NONE,
#endif
  };

  bsp_display_start_with_config(&cfg);
  bsp_display_backlight_on();

  logSection("WiFi");
  esp_err_t wifi_ret = wifi_manager_init();
  if (wifi_ret != ESP_OK) {
    ESP_LOGE(TAG, "WiFi init failed: %s", esp_err_to_name(wifi_ret));
  }

  logSection("Time sync");
  esp_err_t time_ret = time_sync_init();
  if (time_ret != ESP_OK) {
    ESP_LOGE(TAG, "Time sync init failed: %s", esp_err_to_name(time_ret));
  }

  logSection("Mount SD card");
  if (sdcard_mount() == ESP_OK) {
    sdcard_list_dir("/sdcard");
  }

  logSection("Create UI");
  /* Lock the mutex due to the LVGL APIs are not thread-safe */
  bsp_display_lock(0);

  /**
   * Or try out a demo.
   * Don't forget to uncomment header and enable the demos in `lv_conf.h`. E.g. `LV_USE_DEMOS_WIDGETS`
   */
  ui_init();
  ui_screen_start();

  /* Release the mutex */
  bsp_display_unlock();

  logSection("WeatherStation end");
}

void loop()
{
  ESP_LOGI(TAG, "IDLE loop");
  // delay(1000);
}
