```json
{
    "src":"brain",
    "msg_type":"Alive",
    "payload":{
        "subscribe":["MotorEvent","SysEvent"] // mandatory
    }
}
```
```json
{
    "src":"brain", // => IP:PORT
    "dst":null,
    "msg_type":"Alive",
    "payload":{
        "subscribe" :["*"],
    }
}
```
```json
{
    "src":"brain",
    "dst":"esp1",
    "msg_type":"HoverboardCmd",
    "payload": {
        "speed":-200,
        "steer":100
    }
}
```
```json
{
    "src":"esp1",
    "Alive": {
        "subscribe":["MessageType1"]
    }
}
```
```yaml
switch
    - esp1::SysCmd::active=true
```
To make this project stand out to a hobbyist, you need to frame it as the solution to a common frustration: **"How do I make my 10 different ESP32s and Arduinos actually talk to each other and my PC without the code becoming a mess?"**

Here is a punchy, engaging version of your README intro designed to grab attention.

---

## 🤖 TinyLink: The "Glue" for Your Modular Robot

Stop fighting with hardcoded IP addresses and messy wiring. **TinyLink** is a lightweight, hybrid middleware that turns a scattered collection of microcontrollers into a single, cohesive robotic brain.

While heavyweights like ROS 2 can crush a small MCU, TinyLink is built for the "little guys." It uses **UDP Multicast** so your devices can "shout" their presence and find each other automatically—no configuration required. Whether you need the structured reliability of a **Central Broker** for sensor logs or the raw, lightning-fast speed of **Peer-to-Peer** commands for motor timing, TinyLink switches between them seamlessly. It’s the bridge between your low-level hardware and your high-level intelligence.

---

### 🚀 Why you’ll love it:

* **Zero-Config Discovery:** Just power on your ESP32/STM32, and it "checks in" with the rest of the robot automatically.
* **Best of Both Worlds:** Use the **Broker** for easy Python/ROS integration, or go **P2P** when every millisecond of latency counts.
* **Micro-Friendly:** Written to keep your heap usage low and your loop speeds high.
* **Plug-and-Play Sensors:** Add a new sensor node to your robot's body without recompiling the main controller code.

### 🛠 The "Hybrid" Advantage

| Feature | Broker Path (Pub/Sub) | Direct Path (P2P) |
| --- | --- | --- |
| **Best For** | Telemetry, Logging, UI Updates | Real-time Motion, Emergency Stops |
| **Complexity** | Simple (One connection) | Direct (Lowest latency) |
| **Visibility** | Everything can see the data | Private, 1-to-1 communication |

---

### 💡 Project Idea:

Imagine a hexapod where every leg has its own MCU. With this middleware, the legs use **Multicast** to find the body, **P2P** to sync their steps with microsecond precision, and a **Broker** to send battery and terrain data back to your laptop for a real-time dashboard.

**Would you like me to generate a "Features" checklist or a diagram-style ASCII art showing how the Multicast discovery works?**

# Limeros architecture

This middleware implements a **hybrid communication architecture** that balances decentralized discovery with structured data routing. By utilizing **UDP Multicast for autonomous peer discovery**, the system allows microcontrollers to join the network and identify available services without manual configuration. High-frequency telemetry and state changes are channeled through a **central broker for robust Publish/Subscribe messaging**, while time-critical requests can bypass the broker entirely for **direct peer-to-peer (P2P) interaction**. This flexible routing ensures that small devices can maintain a minimal memory footprint while remaining fully integrated into the robot's broader decision-making logic.

---

### Communication Flow

* **Discovery:** UDP Multicast allows nodes to find the broker and other peers dynamically.
* **Pub/Sub:** A centralized broker manages event distribution, ensuring data persistence and easy monitoring.
* **Direct Messaging:** Low-latency requests can be sent directly to a specific device's IP to minimize "hop" delay.

### Why This Works for Small Devices

1. **Reduced Overhead:** MCUs don't need to maintain a constant list of every peer; they only need to know the broker or a specific target.
2. **Reliability:** Using a broker for events ensures that high-level systems (like a Jetson or NUC) can easily aggregate data from dozens of small sensors.
3. **Speed:** Direct P2P messaging allows for "fast-path" commands, such as an emergency stop or a high-speed sync pulse.

Would you like me to create a **comparison table** showing when a message should go through the broker versus when it should be sent peer-to-peer?