#include "CameraUtils.h"

camera_config_t cameraConfig;

esp_err_t initializeCamera() {
    // Set up the camera configuration
    cameraConfig.ledc_channel = LEDC_CHANNEL_0;
    cameraConfig.ledc_timer = LEDC_TIMER_0;
    cameraConfig.pin_d0 = Y2_GPIO_NUM;
    cameraConfig.pin_d1 = Y3_GPIO_NUM;
    cameraConfig.pin_d2 = Y4_GPIO_NUM;
    cameraConfig.pin_d3 = Y5_GPIO_NUM;
    cameraConfig.pin_d4 = Y6_GPIO_NUM;
    cameraConfig.pin_d5 = Y7_GPIO_NUM;
    cameraConfig.pin_d6 = Y8_GPIO_NUM;
    cameraConfig.pin_d7 = Y9_GPIO_NUM;
    cameraConfig.pin_xclk = XCLK_GPIO_NUM;
    cameraConfig.pin_pclk = PCLK_GPIO_NUM;
    cameraConfig.pin_vsync = VSYNC_GPIO_NUM;
    cameraConfig.pin_href = HREF_GPIO_NUM;
    cameraConfig.pin_sccb_sda = SIOD_GPIO_NUM;
    cameraConfig.pin_sccb_scl = SIOC_GPIO_NUM;
    cameraConfig.pin_pwdn = PWDN_GPIO_NUM;
    cameraConfig.pin_reset = RESET_GPIO_NUM;
    cameraConfig.xclk_freq_hz = 4 * 1000000;
    cameraConfig.pixel_format = PIXFORMAT_JPEG;
    cameraConfig.frame_size = FRAMESIZE_QSXGA;
    cameraConfig.jpeg_quality = 12;
    cameraConfig.fb_location = CAMERA_FB_IN_PSRAM;
    cameraConfig.fb_count = 2;
    cameraConfig.grab_mode = CAMERA_GRAB_LATEST;

    esp_err_t error = esp_camera_init(&cameraConfig);
    return error;
}

camera_fb_t* cameraCaptureImage() {
    camera_fb_t* frameBuffer = esp_camera_fb_get();  // Capture the image
    if (!frameBuffer) {
        return nullptr;
    }
    return frameBuffer;
}

void cameraReleaseFrameBuffer(camera_fb_t* frameBuffer) {
    if (frameBuffer) {
        esp_camera_fb_return(frameBuffer);
    }
}
