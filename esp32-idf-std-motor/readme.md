## GUI interface

widgets <=> model <=> middleware

model :
- send value by key ( string containing object and prop id's  )
- callback recv value by key ( string )
- get value by key, time-range
- get value info by key


dst/lm1/motor { target_rpm:3000, max_current_mA:1000 }
src/lm1/motor { target_rpm:3000, measured_rpm:2500, measured_current_mA:253, max_current_mA:1000 }
src/ps4/1 { }

src/charger1 { children=["charger1/battery1","charger1/battery2"], version="1.0.3",build:"22/7/2024 08:21:30"}
src/charger1/battery1  { charge_current_mA : 1000, decharge_current_mA :500 }
src/charger1/battery2 