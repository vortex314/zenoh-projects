enum Protocol {
    Publish{ obj_id:u32, dev_id:u32, data:Vec<u8> },
    Request { obj_id:u32, dev_id:u32, data:Vec<u8> },
    Response { obj_id:u32, dev_id:u32, data:Vec<u8> },
}