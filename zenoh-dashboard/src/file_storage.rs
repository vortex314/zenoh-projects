
pub struct FileStorage {
    path : String,
}

impl FileStorage {
    pub fn new(path: String) -> FileStorage {
        FileStorage {
            path,
        }
    }
    pub fn save(&self, data: &str) -> Result<(), std::io::Error> {
        std::fs::write(&self.path, data)
    }
    pub fn load(&self) -> Result<String, std::io::Error> {
        std::fs::read_to_string(&self.path)
    }
}

impl eframe::Storage for FileStorage {
    fn get_string(&self, key: &str) -> Option<String> {
        match self.load() {
            Ok(data) => Some(data),
            Err(_) => None,
        }
    }

    fn set_string(&mut self, key: &str, value: String) {
        match self.save(&value) {
            Ok(_) => (),
            Err(e) => eprintln!("Error saving data: {}", e),
        }
    }

    fn flush(&mut self) {
        // Nothing to do here
    }
}