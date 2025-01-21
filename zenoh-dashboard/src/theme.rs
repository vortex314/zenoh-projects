pub struct Theme {
    pub title_background: egui::Color32,
    pub title_text: egui::Color32,

}

pub static  THEME: Theme = Theme {
     title_background: egui::Color32::from_rgb(10, 010, 255),
    title_text: egui::Color32::from_rgb(255, 255, 255),
};