struct Cell {
    src : String,
    field : String,
    last_update : u64,
    value : Value,
}

impl Cell {
    fn expired(&self) -> bool {
        false
    }
}