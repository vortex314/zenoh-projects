use decode::Error;
use minicbor::*;
use minicbor::data::Token;

pub fn payload_encode<X>( v: X) -> Vec<u8>
where
    X: Encode<()>,
{
    let mut buffer = Vec::<u8>::new();
    let mut encoder = Encoder::new(&mut buffer);
    let _x = encoder.encode(v);
    _x.unwrap().writer().to_vec()
}

pub fn payload_decode<'a,T>(v: &'a Vec<u8>) -> Result<T, decode::Error>
where T : Decode<'a,()>
{
    let mut decoder = Decoder::new(v);
    decoder.decode::<T>()
}


pub fn payload_display(v: &Vec<u8>) -> String {
    let line:String  = v.iter().map(|b| format!("{:02X} ", b)).collect();
    let s = format!("{}", minicbor::display(v.as_slice()));
    if s.len() == 0 {
        line
    } else {
        s
    }
}


pub fn payload_as_f64 (payload: &Vec<u8>) -> Result<f64, decode::Error> {
    let mut decoder = Decoder::new(payload);
    let v =  decoder.tokens().collect::<Result<Vec<Token>, _>>()?;
    match v[0] {
        Token::F16(f) => Ok(f as f64),
        Token::F32(f) => Ok(f as f64),
        Token::F64(f) => Ok(f),
        Token::I16(i) => Ok(i as f64),
        Token::I32(i) => Ok(i as f64),
        Token::I64(i) => Ok(i as f64),
        Token::U16(i) => Ok(i as f64),
        Token::U32(i) => Ok(i as f64),
        Token::U64(i) => Ok(i as f64),
        Token::I8(i) => Ok(i as f64),
        Token::U8(i) => Ok(i as f64),
        Token::Bool(b) => Ok(if b { 1.0 } else { 0.0 }),
        _ => Err(Error::type_mismatch(decoder.datatype().unwrap())),
    }
}

pub fn payload_as_bool (payload: &Vec<u8>) -> Result<bool, decode::Error> {
    let mut decoder = Decoder::new(payload);
    let v =  decoder.tokens().collect::<Result<Vec<Token>, _>>()?;
    if v.len() != 1 {
        return Err(Error::end_of_input());
    }
    match v[0] {
        Token::Bool(b) => Ok(b),
        _ => Err(Error::end_of_input()),
    }
}