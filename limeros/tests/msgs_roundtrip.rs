use limeros::msgs::{Alive, Msg, SysEvent, UdpMessage};





#[test]
fn udp_message_cbor_roundtrip_preserves_payload_bytes() {
    let msg = UdpMessage {
        dst: Some("dst".to_string()),
        src: Some("src".to_string()),
        msg_type: Some("SysEvent".to_string()),
        payload: Some(vec![0, 1, 2, 3, 255]),
    };

    let bytes = msg.cbor_serialize().unwrap();
    let decoded = UdpMessage::cbor_deserialize(&bytes).unwrap();

    assert_eq!(decoded.dst, msg.dst);
    assert_eq!(decoded.src, msg.src);
    assert_eq!(decoded.msg_type, msg.msg_type);
    assert_eq!(decoded.payload, msg.payload);
}

#[test]
fn sys_event_json_roundtrip_preserves_fields() {
    let mut e = SysEvent::default();
    e.utc = Some(123);
    e.uptime = Some(456);
    e.cpu_board = Some("linux-x86".to_string());

    let bytes = e.json_serialize().unwrap();
    let decoded = SysEvent::json_deserialize(&bytes).unwrap();

    assert_eq!(decoded.utc, e.utc);
    assert_eq!(decoded.uptime, e.uptime);
    assert_eq!(decoded.cpu_board, e.cpu_board);
}
