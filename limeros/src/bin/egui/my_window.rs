use eframe::egui;
use anyhow::Result;

pub trait MyWindow {
     fn name(&self) -> &'static str;
     fn show(&mut self, ui: &mut egui::Ui)-> Result<()>;
     fn is_closed(&self) -> bool;
}