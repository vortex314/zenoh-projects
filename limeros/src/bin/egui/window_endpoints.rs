use crate::my_window::{ MyWindow};
use dashmap::DashMap;
use eframe::egui;
use limeros::{
    Endpoint, UdpMessage, UdpNode,
};
use anyhow::Result;
use log::info;
use std::{sync::Arc, time::Instant};

pub struct WindowEndpoints {
   node : Arc<UdpNode>  ,
   endpoints: Arc<DashMap<String, Endpoint>>,
}   

impl WindowEndpoints {
    pub fn new(node: Arc<UdpNode>, endpoints: Arc<DashMap<String, Endpoint>>) -> Self {
        Self { node, endpoints }
    }
}

impl MyWindow for WindowEndpoints {
    fn name(&self) -> &'static str {
        "Endpoints"
    }

    fn show(&mut self, ui: &mut egui::Ui) -> Result<()> {
        egui::Window::new("Endpoints")
            .open(&mut true)
            .resizable([true, true])
            .constrain_to(ui.available_rect_before_wrap())
            .show(ui.ctx(), |ui| {
                egui::Grid::new("endpoint_grid")
                    .striped(true)
                    .show(ui, |ui| {
                        ui.label("Peer ID");
                        ui.label("Address");
                        ui.label("Subscribe");
                        ui.label("Publish");
                        ui.label("Services");
                        ui.label("Last Seen");
                        ui.end_row();

                        for entry in self.endpoints.iter() {
                            let ep = entry.value();
                            let secs_ago =
                                Instant::now().duration_since(ep.last_seen).as_millis() / 1000;

                            ui.label(entry.key());
                            ui.label(ep.addr.to_string());
                            ui.label(format!("{:?}", ep.subscribe));
                            ui.label(format!("{:?}", ep.publish));
                            ui.label(format!("{:?}", ep.services));
                            ui.label(format!("{} sec ago", secs_ago));
                            ui.end_row();
                        }
                    });
            });
            Ok(())
    }

    fn is_closed(&self) -> bool {
        false
    }

}

