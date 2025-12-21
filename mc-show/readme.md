# Multicast UDP for scouting and UDP Unicast for events and requests/reply

- A domain is identified by a multicast address, example : House , lawnmower
- 

## Scouting messages

- Announce < endpoint , [message_types]>
- Dictionary ["Key1","Key2",...,"KeyN"] => for FNV hashes 
- All parties can listen to this and replace DNS 

## Event broker 

## To broker
- Subscribe <SourceIp,SourcePort> <dst:pc1/broker,src:esp1/light> { message_type : SwitchOn }
- Unsubscribe

## Scouting Broker 
- <SourceIp,SourcePort> <"","pc1/broker","Announce",["Subscribe","Unsubscribe"]

- Routing command to client
<dstIp,DstPort> <sourceIp,sourcePort> dst : esp1/switch , src : esp2/light msg_type: Connect payload:{ msg_type_in: SwitchOn , msg_type_out : LightOn }

# to any client

## Why UDP
- Multicast is slowed down to basic rate on wifi
- too many mu;ticast will impact too many clients