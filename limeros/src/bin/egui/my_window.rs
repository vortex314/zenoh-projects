use eframe::egui;
use limeros::UdpMessage;
pub trait MyWindow {
     fn name(&self) -> &'static str;
     fn show(&mut self, ui: &mut egui::Ui);
     fn ui(&mut self, ui: &mut egui::Ui);
     fn on_message(&mut self, udp_message: &UdpMessage);
}