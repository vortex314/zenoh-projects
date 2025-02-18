# ESP32 Camera via Zenoh 

<img src="doc/zenoh-cam.jpeg" width="400" height="400" />

## Features
- Send JPEG images ( VGA resolution or higher ) via Zenoh to broker 
- Receive commands to activate light on camera 
- Send properties of wifi, zenoh , sys actors

```C++
struct CameraMsg : public Serializable
{
    std::optional<Bytes> image = std::nullopt;
    std::optional<bool> light = std::nullopt;
};
```


![Icon](doc/zenoh-cam.jpeg)