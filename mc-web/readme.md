# Architecture

# Actor Model

# Message model

- msg["Kp"].set(\_Kp);
- msg["rpm"]=2300.6;
-

# Device model

```json
{
    "src":"esp1/motor",
    "pub":{
            "rpm_target":23333.4,
            "Kp":1.23,
            "Ki":0.001,
            "Kd":-0.0001,
        },
    "info":{
            "rpm_target": {
                "unit":"RPM",
                "desc":"The target setting of the RPM of the cutter",
                "mode":"RW",
                }
            }
    }
}
```

```json
{
    "dst":"esp1/motor",
    "pub":{
            "rpm_target":3000,
            "Kp":1.23,
            "Ki":0.001,
            "Kd":-0.0001,
        },
    "info":{
            "rpm_target": {
                "unit":"RPM",
                "desc":"The target setting of the RPM of the cutter",
                }
            }
    }
}
```

```json
{
    "dst":"esp1/motor",
    "src":"brain/reply",
    "req":{
            "rpm_target":3000,
            "Kp":1.23,
            "Ki":0.001,
            "Kd":-0.0001,
        },
    "rep":{
            "rc":0,
            "msg":"all ok"
    }
```
- dst : None, src : Some => publish
- dst : Some , src : None => tell
- dst : Some, src : Some => request/reply