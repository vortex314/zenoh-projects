#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
#![allow(unused_imports)]
#![allow(dead_code)]
//#![deny(elided_lifetimes_in_paths)]
use std::sync::Arc;

use egui::Visuals;
//use egui_tiles::Tile::Pane;
// hide console window on Windows in release

use anyhow::Result;

mod actor_zenoh;
use actor_zenoh::{Actor, ZenohActor};
mod logger;
use log::info;
use logger::init;
use tokio::sync::Mutex;

mod value;
use value::Value;
mod pane;
use pane::{Pane, PaneWidget};
mod widget_text;
use widget_text::WidgetText;

struct TreeBehavior {}

impl egui_tiles::Behavior<Pane> for TreeBehavior {
    fn tab_title_for_pane(&mut self, pane: &Pane) -> egui::WidgetText {
        pane.widget
            .try_lock()
            .map(|w| w.title())
            .unwrap_or_default()
            .into()
    }

    fn pane_ui(
        &mut self,
        ui: &mut egui::Ui,
        _tile_id: egui_tiles::TileId,
        pane: &mut Pane,
    ) -> egui_tiles::UiResponse {
        let rect = ui.max_rect();
        let mut button_rect = ui.max_rect();
        button_rect.max.y = button_rect.min.y + 20.0;

        let response = ui.scope(|ui| {
            /*ui.visuals_mut().widgets.inactive.bg_fill = egui::Color32::from_rgb(0, 0, 255);
            ui.visuals_mut().widgets.active.bg_fill = egui::Color32::from_rgb(0, 0, 255);
            ui.visuals_mut().widgets.open.bg_fill = egui::Color32::from_rgb(0, 0, 255);
            ui.visuals_mut().widgets.inactive.bg_fill = egui::Color32::from_rgb(0, 0, 255);
            ui.visuals_mut().widgets.hovered.bg_fill = egui::Color32::from_rgb(0, 0, 255);
            ui.visuals_mut().widgets.noninteractive.bg_fill = egui::Color32::from_rgb(0, 0, 255);*/
            ui.visuals_mut().override_text_color = Some(egui::Color32::from_rgb(255, 255, 255));
            let response = if ui
                .put(
                    button_rect,
                    egui::Button::new(self.tab_title_for_pane(pane))
                        .sense(egui::Sense::drag())
                        .fill(egui::Color32::from_rgb(0, 0, 255)),
                )
                .drag_started()
            {
                egui_tiles::UiResponse::DragStarted
            } else {
                egui_tiles::UiResponse::None
            };
            response
        });

        ui.add(egui::widgets::Label::new(format!(
            "Rect [{},{}] , [{},{}] ",
            rect.left(),
            rect.top(),
            rect.right(),
            rect.bottom(),
        )));
        let _ = pane.widget.try_lock().map(|mut w| w.show(ui));
        response.inner
    }
}

#[tokio::main(flavor = "multi_thread", worker_threads = 2)]
async fn main() -> Result<(), eframe::Error> {
    logger::init();
    info!("Starting dashboard ");

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([640.0, 480.0]),
        ..Default::default()
    };

    let mut tree = create_tree();

    let mut actor_zenoh = ZenohActor::new();

    let mut pane_widgets = vec![];
    for (tile_id, tile_pane) in tree.tiles.iter() {
        match tile_pane {
            egui_tiles::Tile::Pane(pane) => {
                let widget = pane.widget.clone();
                pane_widgets.push(widget.clone());
            }
            egui_tiles::Tile::Container(container) => {
                info!(" Container {:?} ", container.kind());
            }
        };
    }

    actor_zenoh.add_listener(move |_event| match _event {
        actor_zenoh::ZenohEvent::Publish { topic, payload } => {
            let r = Value::from_cbor(payload.to_vec());
            if let Ok(value) = r {
                info!(" RXD {} :{} ", topic, value);
                for pane_widget in pane_widgets.iter_mut() {
                    let _ = pane_widget
                        .try_lock()
                        .map(|mut w| w.process_data(topic.clone(), &value));
                }
            } else {
                info!(
                    "Error decoding payload {:?} => {} : {:02X?}",
                    r.err().unwrap().to_string(),
                    topic,
                    payload
                );
            }
        }
        _ => {}
    });
    tokio::spawn(async move {
        actor_zenoh.run().await;
    });

    eframe::run_simple_native("Zenoh ", options, move |ctx, _frame| {
        egui::CentralPanel::default().show(ctx, |ui| {
            ctx.set_visuals(Visuals::light());
            let mut behavior = TreeBehavior {};
            tree.ui(&mut behavior, ui);
        });
    })
}

fn create_tree() -> egui_tiles::Tree<Pane> {
    let mut tiles = egui_tiles::Tiles::<Pane>::default();

    let mut tabs = vec![];
    tabs.push({
        let mut children: Vec<egui_tiles::TileId> = vec![];
        children.push(tiles.insert_pane(Pane::new(WidgetText::new(
            "Sys".to_string(),
            "src/lm1/sys".to_string(),
        ))));
        children.push(tiles.insert_pane(Pane::new(WidgetText::new(
            "Wifi".to_string(),
            "src/lm1/wifi".to_string(),
        ))));
        children.push(tiles.insert_pane(Pane::new(WidgetText::new(
            "Zenoh".to_string(),
            "src/lm1/zenoh".to_string(),
        ))));
        tiles.insert_horizontal_tile(children)
    });

    let root = tiles.insert_tab_tile(tabs);

    egui_tiles::Tree::new("my_tree", root, tiles)
}
