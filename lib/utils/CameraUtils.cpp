#include <esp_camera.h>

#include <CameraPins.h>

camera_config_t static_camera_config;

esp_err_t initCameraDefault(){
    // Set up the camera configuration
    static_camera_config.ledc_channel = LEDC_CHANNEL_0;
    static_camera_config.ledc_timer = LEDC_TIMER_0;
    static_camera_config.pin_d0 = Y2_GPIO_NUM;
    static_camera_config.pin_d1 = Y3_GPIO_NUM;
    static_camera_config.pin_d2 = Y4_GPIO_NUM;
    static_camera_config.pin_d3 = Y5_GPIO_NUM;
    static_camera_config.pin_d4 = Y6_GPIO_NUM;
    static_camera_config.pin_d5 = Y7_GPIO_NUM;
    static_camera_config.pin_d6 = Y8_GPIO_NUM;
    static_camera_config.pin_d7 = Y9_GPIO_NUM;
    static_camera_config.pin_xclk = XCLK_GPIO_NUM;
    static_camera_config.pin_pclk = PCLK_GPIO_NUM;
    static_camera_config.pin_vsync = VSYNC_GPIO_NUM;
    static_camera_config.pin_href = HREF_GPIO_NUM;
    static_camera_config.pin_sccb_sda = SIOD_GPIO_NUM;
    static_camera_config.pin_sccb_scl = SIOC_GPIO_NUM;
    static_camera_config.pin_pwdn = PWDN_GPIO_NUM;
    static_camera_config.pin_reset = RESET_GPIO_NUM;
    static_camera_config.xclk_freq_hz = 4 * 1000000;
    static_camera_config.pixel_format = PIXFORMAT_JPEG;

    static_camera_config.frame_size = FRAMESIZE_QSXGA;
    static_camera_config.jpeg_quality = 12;
    static_camera_config.fb_location = CAMERA_FB_IN_PSRAM;
    static_camera_config.fb_count = 2;
    static_camera_config.grab_mode = CAMERA_GRAB_LATEST;


    esp_err_t err = esp_camera_init(&static_camera_config);
    return err;
}

