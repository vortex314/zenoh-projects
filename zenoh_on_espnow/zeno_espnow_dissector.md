To capture ESP-NOW packets and use a dissector to display them in Wireshark, you'll need to follow a few steps. ESP-NOW is a proprietary protocol developed by Espressif for their ESP8266 and ESP32 devices, so capturing these packets requires some specific setup. Here's a general approach:



1. Capture ESP-NOW packets:
   - ESP-NOW uses Wi-Fi, but in a proprietary way. You'll need a Wi-Fi adapter capable of monitor mode.
   - Put your Wi-Fi adapter into monitor mode. On Linux, you might use a command like:
     ```
     sudo airmon-ng start wlan0
     ```

2. Use Wireshark to capture packets:
   - Start Wireshark and select your monitor mode interface.
   - Apply a filter to focus on ESP-NOW packets. ESP-NOW uses action frames, so you might use a filter like:
     ```
     wlan.fc.type_subtype == 0x0d
     ```

3. Develop or obtain an ESP-NOW dissector:
   - As of my last update, Wireshark didn't have a built-in dissector for ESP-NOW.
   - You might need to write a custom Lua dissector for ESP-NOW.

4. Load the dissector in Wireshark:
   - Go to Wireshark's preferences.
   - Under "Protocols," find the option to add a new Lua script.
   - Load your ESP-NOW dissector script.

5. Analyze the packets:
   - With the dissector loaded, Wireshark should now be able to interpret ESP-NOW packets.
   - You should see detailed breakdowns of the ESP-NOW packet structure.

Here's a basic outline of what an ESP-NOW Lua dissector might look like:

```lua
-- ESP-NOW dissector

esp_now_protocol = Proto("ESP-NOW","ESP-NOW Protocol")

-- ESP-NOW fields
local f_type = ProtoField.uint8("esp_now.type", "Type", base.HEX)
local f_payload = ProtoField.bytes("esp_now.payload", "Payload")

esp_now_protocol.fields = {f_type, f_payload}

function esp_now_protocol.dissector(buffer, pinfo, tree)
    length = buffer:len()
    if length == 0 then return end

    pinfo.cols.protocol = esp_now_protocol.name

    local subtree = tree:add(esp_now_protocol, buffer(), "ESP-NOW Protocol Data")
    
    subtree:add(f_type, buffer(0,1))
    subtree:add(f_payload, buffer(1))
end

-- Register the dissector
local wifi_table = DissectorTable.get("wlan.fc.type_subtype")
wifi_table:add(0x0d, esp_now_protocol)
```

This is a very basic dissector and would need to be expanded to handle all the fields in an ESP-NOW packet.

Remember that capturing and analyzing proprietary protocols may have legal implications, so ensure you have the right to do so in your jurisdiction and use case.

Would you like more details on any part of this process? Or do you need help with a specific aspect of capturing or dissecting ESP-NOW packets?