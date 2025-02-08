
#include <camera_actor.h>

/*
ESPHOME setup that works
esp32_camera:
  name: $device_name
  external_clock:
    pin: GPIO0
    frequency: 20MHz
  i2c_pins:
    sda: GPIO26
    scl: GPIO27
  data_pins: [GPIO5, GPIO18, GPIO19, GPIO21, GPIO36, GPIO39, GPIO34, GPIO35]
  vsync_pin: GPIO25
  href_pin: GPIO23
  pixel_clock_pin: GPIO22
  power_down_pin: GPIO32
  resolution: 1024x768
  idle_framerate: 0.1fps
  jpeg_quality: 10

*/
#define CAM_PIN_PWDN 32  // power down
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 0  // external clock
#define CAM_PIN_SIOD 26  // SDA
#define CAM_PIN_SIOC 27  // SCL

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

void CameraActor::process_image(int width, int height, int format, uint8_t *data, size_t len)
{
    INFO("Processing image: %dx%d, format=%d, len=%d", width, height, format, len);
    _camera_msg.image = Bytes(data, data + len);
    _camera_msg.light = std::nullopt;
    PublishSerdes serdes(_camera_msg);
    emit(CameraEvent{.serdes = serdes});
}

Res CameraActor::camera_init()
{
    // power up the camera if PWDN pin is defined
    if (CAM_PIN_PWDN != -1)
    {
        gpio_set_direction((gpio_num_t)CAM_PIN_PWDN, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)CAM_PIN_PWDN, 0);
    }

    // initialize the camera
    CHECK(esp_camera_init(&_camera_config));

    return Res::Ok();
}

Res CameraActor::camera_capture()
{
    // acquire a frame
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        return Res::Err(1, "Camera Capture Failed");
    }
    // replace this with your own function
    process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);

    // return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
    return Res::Ok();
}

CameraActor::CameraActor() : CameraActor("sys", 4096, 5, 5) {}

CameraActor::CameraActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<CameraEvent, CameraCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(1000);

    _camera_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = 16000000, // EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode, was 20_000_000
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE_UXGA,   // QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

        .jpeg_quality = 12,                  // 0-63, for OV series camera sensors, lower number means higher quality
        .fb_count = 1,                       // When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
        .fb_location = CAMERA_FB_IN_PSRAM,   // CAMERA_FB_IN_RAM or CAMERA_FB_IN_PSRAM
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY, // CAMERA_GRAB_LATEST. Sets when buffers should be filled
                                             //     .fb_location = CAMERA_FB_IN_PSRAM,   // CAMERA_FB_IN_RAM or CAMERA_FB_IN_PSRAM
                                             //     .sccb_i2c_port = I2C_NUM_0,          // I2C port number

    };
}

void CameraActor::on_cmd(CameraCmd &cmd)
{
}

void CameraActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        INFO("Timer 1 : Publishing Camera properties");
        auto r = camera_capture();
        if (r.is_err())
        {
            ERROR("Camera capture failed: [%d] %s",r.rc(), r.msg().c_str());
        }
    }
    else
    {
        INFO("Unknown timer id: %d", id);
    }
}

void CameraActor::on_start(void)
{
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    INFO("Starting Camera actor");
    Res r = camera_init();
    if (r.is_err())
    {
        ERROR("Camera init failed:[%d] %s",r.rc(), r.msg().c_str());
    }
}

CameraActor::~CameraActor()
{
    INFO("Stopping Camera actor");
}

Res CameraMsg::serialize(Serializer &ser)
{
    ser.reset();
    ser.array_begin();
    ser.serialize(image);
    ser.serialize(light);
    ser.array_end();
    return Res::Ok();
}

Res CameraMsg::deserialize(Deserializer &des)
{
    des.array_begin();
    des.deserialize(image);
    des.deserialize(light);
    des.array_end();
    return Res::Ok();
}
