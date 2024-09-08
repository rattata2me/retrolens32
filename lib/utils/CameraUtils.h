#ifndef RETROLENS_CAMERA_UTILS_H
#define RETROLENS_CAMERA_UTILS_H

#include <esp_camera.h>

#include "CameraPins.h"

/**
 * @brief Camera configuration structure.
 */
extern camera_config_t cameraConfig;

/**
 * @brief Initialize the camera with the specified configuration.
 * 
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t initializeCamera();

/**
 * @brief Capture an image using the camera.
 * 
 * @return camera_fb_t* Pointer to the frame buffer containing the captured image, or nullptr on failure.
 */
camera_fb_t* cameraCaptureImage();

/**
 * @brief Release the frame buffer after processing the captured image.
 * 
 * @param frameBuffer Pointer to the frame buffer to be released.
 */
void cameraReleaseFrameBuffer(camera_fb_t* frameBuffer);

#endif // RETROLENS_CAMERA_UTILS_H