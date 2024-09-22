
type Map<K,V> = alloc::BtreeMap<K,V>;
type PropKey = u8;
type ObjectId = u32;
type MsgType = u8;
type MsgId = u16;
type Bytes = Vec<u8>;

struct Value {
    u16 : u16,
    u32 : u32,
    u64 : u64,
    i16 : i16,
    i32 : i32,
    i64 : i64,
    f32 : f32,
    f64 : f64,
    bool : bool,
    string : String,
    bytes : Vec<PropKey>,
}

enum ValueType {
    U8,U16,U32,U64,
    I8,I16,I32,I64,
    F32,F64,
    Bool,
    String,
    Bytes,
}

enum ValueMode {
    Get =1 ,
    Set  =2,
    Pub =  4,
}

struct ObjectDescriptor {
    name : String,
    id : u32,
    desc : String,
    props : Map<PropKey, PropDescriptor>,
}

struct PropertyDescriptor {
    name : String,
    id : u32,
    desc : String,
    props : Map<PropKey, PropDescriptor>,
    value_type : ValueType,
    value_mode : ValueMode,
}

const object_desc:ObjectDescriptor = ObjectDescriptor { name:"ps4", id:fnv("ps4"), desc:"Playstation 4", vec![
    PropertyDescriptor { id:0, name :"left_axis_x",desc:"Left Joystick X value",value_type:ValueType::I16, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:1, name :"left_axis_y",desc:"Left Joystick Y value",value_type:ValueType::I16, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:2, name :"right_axis_x",desc:"Right Joystick X value",value_type:ValueType::I16, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:3, name :"right_axis_y",desc:"Right Joystick Y value",value_type:ValueType::I16, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:4, name :"button_x",desc:"Button X value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:5, name :"button_y",desc:"Button Y value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:6, name :"button_a",desc:"Button A value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:7, name :"button_b",desc:"Button B value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:8, name :"button_l1",desc:"Button L1 value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:9, name :"button_l2",desc:"Button L2 value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:10, name :"button_r1",desc:"Button R1 value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:11, name :"button_r2",desc:"Button R2 value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:12, name :"button_start",desc:"Button Start value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:13, name :"button_select",desc:"Button Select value",value_type:ValueType::Bool, value_mode:ValueMode::Get | ValueMode::Pub },
    PropertyDescriptor { id:14, name :"rumble",desc:"Button PS value",value_type:ValueType::u8, value_mode:ValueMode::Set  },
    PropertyDescriptor { id:15 , name :"led_green",desc:"LED Green value",value_type:ValueType::U8, value_mode:ValueMode::Set  },
    PropertyDescriptor { id:16 , name :"led_red",desc:"LED Red value",value_type:ValueType::U8, value_mode:ValueMode::Set  },
    PropertyDescriptor { id:17 , name :"led_blue",desc:"LED Blue value",value_type:ValueType::U8, value_mode:ValueMode::Set  },
]};



enum ReturnCode {
    Ok = 0,
    Error = 1,
    NotFound = 2,
    Invalid = 3,
}

enum MsgType {
    Alive = 0,
    Get = 1,
    Set = 2,
    Name = 3,
    Desc = 4,
    Pub = 5,
    Sub = 6,
}


struct Header {
    from : Option<ObjectId>,
    to : Option<ObjectId>,
    msg_id : Option<MsgId>,
    msg_type : MsgType,
}


// don't expect any reply , for non-receiving objects
enum OneWay {
    PubMap {  values : Map<PropKey,Value> },
    PubVec {  values : Vec<Value> },
    PubReg { prop_id : Option<PropKey>, name : String, desc : Option<String> },    
    Alive { },
}
// expect a reply
enum RequestMsg {
    Get {  keys : Vec<PropKey> },
    Set {  setters : Map<PropKey, Value> },
    Name {  prop_keys : Option<Vec<PropKey>> },
    Desc {  prop_keys : Option<Vec<PropKey>> },
}
// reply    
enum ReplyMsg {
    Get {    values : Map<PropKey,Value> },
    Set {  setters : Map<PropKey, Value> },
    Name {  prop_names : Map<PropKey,String> },
    Desc {  prop_descs : Map<PropKey,String> },
    Error {  erc: ReturnCode, error : String },
}

fn route_request(msg : RequestMsg) -> Result<ReplyMsg> {
    match msg {
        RequestMsg::Get { header, keys } => {
            Ok ( ReplyMsg::Error { header.error_reply, erc : ReturnCode::NotFound, error : "Not supported".to_string() } )
        },
        RequestMsg::Set { header, setters } => {
            Ok ( ReplyMsg::Error { header.error_reply, erc : ReturnCode::NotFound, error : "Not supported".to_string() } )
        },
        RequestMsg::PubMap { header, values } => {
            Ok ( ReplyMsg::Error { header.error_reply, erc : ReturnCode::NotFound, error : "Not supported".to_string() } )
        },
        RequestMsg::PubVec { header, values } => {
            Ok ( ReplyMsg::Error { header.error_reply, erc : ReturnCode::NotFound, error : "Not supported".to_string() } )
        },
        RequestMsg::Name { header, prop_keys } => {
            Ok ( ReplyMsg::Error { header.error_reply, erc : ReturnCode::NotFound, error : "Not supported".to_string() } )
        },
        RequestMsg::Desc { header, prop_keys } => {
            Ok ( ReplyMsg::Error { header.error_reply, erc : ReturnCode::NotFound, error : "Not supported".to_string() } )
        },
    }
}

struct PubSubService {
    objects : Map<ObjectId, ObjectDescriptor>,
    gateway : Map<String, ObjectId>,
    cursor : Option<(ObjectId, PropKey)>,
}

impl PubSubService {
    fn register_object(&mut self, obj : ObjectDescriptor,handler : Box<dyn Handler<Msg>> ) -> Result<()> {
        let id = obj.id;
        if self.objects.contains_key(&id) {
            return Err("Object already registered".to_string());
        }
        self.objects.insert(id, obj);
        Ok(())
    }
    fn unregister_object(&mut self, id : u32) -> Result<()> {
        if self.objects.contains_key(&id) {
            self.objects.remove(&id);
            Ok(())
        } else {
            Err("Object not found".to_string())
        }
    }
    fn publish_next(&mut self) -> Result<()> {
        if let Some((id, prop_id)) = self.cursor {
            if let Some(obj) = self.objects.get(&id) {
                if let Some(prop) = obj.props.get(&prop_id) {
                    let header = Header { from : Some(id), to : None, msg_id : 0, msg_type : MsgType::Pub };
                    let msg = OneWay::PubMap { header : header, values : vec![(prop_id, prop.value.clone())].into_iter().collect() };
                    self.route_one_way(msg);
                }
            }
        }
        Ok(())
    }
    fn route_one_way(&mut self, msg : OneWay) -> Result<()> {
        match msg {
            OneWay::PubMap { header, values } => {
                for (id, obj) in self.objects.iter() {
                    if let Some(prop_id) = header.to {
                        if let Some(prop) = obj.props.get(&prop_id) {
                            let header = Header { from : Some(id), to : None, msg_id : 0, msg_type : MsgType::Pub };
                            let msg = OneWay::PubMap { header : header, values : vec![(prop_id, prop.value.clone())].into_iter().collect() };
                            self.route_one_way(msg);
                        }
                    } else {
                        let header = Header { from : Some(id), to : None, msg_id : 0, msg_type : MsgType::Pub };
                        let msg = OneWay::PubMap { header : header, values : values.clone() };
                        self.route_one_way(msg);
                    }
                }
            },
            OneWay::PubVec { header, values } => {
                for (id, obj) in self.objects.iter() {
                    if let Some(prop_id) = header.to {
                        if let Some(prop) = obj.props.get(&prop_id) {
                            let header = Header { from : Some(id), to : None, msg_id : 0, msg_type : MsgType::Pub };
                            let msg = OneWay::PubVec { header : header, values : vec![prop.value.clone()] };
                            self.route_one_way(msg);
                        }
                    } else {
                        let header = Header { from : Some(id), to : None, msg_id : 0, msg_type : MsgType::Pub };
                        let msg = OneWay::PubVec { header : header, values : values.clone() };
                        self.route_one_way(msg);
                    }
                }
            },
            OneWay::PubReg { header, id, prop_id, name, desc } => {
                if let Some(obj) = self.objects.get(&id) {
                    if let Some(prop_id) = prop_id {
                        if let Some(prop) = obj.props.get(&prop_id) {
                            let header = Header { from : Some(id), to : None, msg_id : 0, msg_type : MsgType::Pub };
                            let msg = OneWay::PubReg { header : header, id : id, prop_id : Some(prop), name : obj.name.clone(), desc : obj.desc.clone() };
                            self.route_one_way(msg);



}

/*

MQTT struct 
dst/obj_id/prop_id -> value                     ==> Pub
dst/obj_id -> "set" : { prop_id : value }       ==> Set
dst/obj_id -> "get" : [ prop_id ]               ==> Get
dst/obj_id -> "name" : [ prop_id ]              ==> Name
dst/obj_id -> "desc" : [ prop_id ]              ==> Desc
src/obj_id -> "alive"                           ==> Alive
src/obj_id -> "pub" : { prop_id : value }       ==> Pub
src/obj_id -> "pub" : [ value ]                 ==> Pub
src/object_name/prop_name -> value              ==> Pub
src/object_name -> { prop_name : value, prop_name1: value1 }       => Pub
dst/object_name/prop_name/set -> value              ==> Set
dst/object_name/prop_name/get -> value              ==> Get
dst/object_name/prop_name/name -> value             ==> Name
dst/object_name/prop_name/desc -> value             ==> Desc
dst/object_name/prop_name -> value             ==> Desc

dst/object_name/setReply -> objectId,propId,msg_id,value              ==> Set
dst/object_name/getReply -> value              ==> Get
dst/object_name/nameReply -> value             ==> Name
dst/object_name/descReply -> value             ==> Desc

Gateway -> obj_id dictionary

Main States : Mow, Charge , Sleep, Error, Walk
Sub States : Mow
         - Slow , Fast ( Line from-to )
         - AvoidObstacle
         - EmptyBattery -> Walk to Charger, 
Walk :
    - Line from-to
    - AvoidObstacle 
    - EmptyBattery -> Walk to Charge Point  or walk to Mowing Point
Sleep :
    - Wakeup -> Mow
    - Charge -> Charge
    - Error -> Error
Error :
    - Reset -> Mow
    - Charge -> Charge
    - Error -> Error

*/
Displaying Afrekening verkoper 12.9.pdf.