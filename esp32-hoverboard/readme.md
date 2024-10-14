# ESP32-Hoverboard drive

connect an esp32 to an hacked hoverboard driver via UART commands 
- HB --> ESP32 : payload is CRC checked , COBS decoded and send as such in broadcast mode
- ESP32 --> HB : ESP32 receives ESPNOW message, looks for 2 props : speed and steer and sends it via UART to HB
In the direct drive mode , it accepts directives from a PS4 controller and send the joystick commands to the HB.