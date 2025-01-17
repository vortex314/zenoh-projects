#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
#![allow(unused_imports)]
#![allow(dead_code)]
use egui::Visuals;
//use egui_tiles::Tile::Pane;
// hide console window on Windows in release

use serde::de::DeserializeOwned;
use serde::{Deserialize, Serialize};

use anyhow::Result;

mod actor_zenoh;
use actor_zenoh::{Actor, ActorZenoh};
mod logger;
use log::info;
use logger::init;
use tokio::sync::Mutex;

trait PaneWidget: std::fmt::Debug + Send {
    fn show(&mut self, ui: &mut egui::Ui);
    fn title(&self) -> String;
    fn process_data(&mut self, decoder: &mut minicbor::Decoder) -> ();
}

#[derive(Debug)]
struct Pane {
    widget: Mutex<Box<dyn PaneWidget>>,
}

impl Pane {
    fn new(widget: impl PaneWidget + 'static) -> Self {
        Self {
            widget: Mutex::new(Box::new(widget)),
        }
    }
}

#[derive(Debug)]
struct TextPane {
    text: String,
}

impl PaneWidget for TextPane {
    fn show(&mut self, ui: &mut egui::Ui) {
        ui.label(&self.text);
        ui.label("Hello, TextPane");
    }

    fn title(&self) -> String {
        self.text.clone()
    }

    fn process_data(&mut self, decoder: &mut minicbor::Decoder) {
        info!("Processing data");
        self.text = decoder.decode::<String>().unwrap();
    }
}

struct TreeBehavior {}

impl egui_tiles::Behavior<Pane> for TreeBehavior {
    fn tab_title_for_pane(&mut self, pane: &Pane) -> egui::WidgetText {
        pane.widget.try_lock().unwrap().title().into()
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
                    egui::Button::new("Hello, world!")
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
        pane.widget.try_lock().unwrap().show(ui);
        response.inner
    }
}

#[derive(Debug, Serialize, Deserialize)]
struct Data {
    value: String,
}

struct ShareableData {
    data: Mutex<Data>,
}

#[tokio::main(flavor = "multi_thread", worker_threads = 2)]
async fn main() -> Result<(), eframe::Error> {
    logger::init();
    info!("Starting dashboard ");

    let data = ShareableData {
        data: Mutex::new(Data {
            value: "Hello, world!".to_string(),
        }),
    };

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([640.0, 480.0]),
        ..Default::default()
    };

    let mut tree = create_tree();

    let static_data = Box::leak(Box::new(data));

    let mut actor_zenoh = ActorZenoh::new();
    actor_zenoh.add_listener(|_event| {
        info!("Event {:?}", _event);
        static_data.data.try_lock().unwrap().value = "Hello, Zenoh 2!".to_string();
        info!("Event {:?}", static_data.data.try_lock().unwrap().value);
        for (tileid,tile) in tree.tiles.iter_mut() {
            match tile {
                egui_tiles::Tile::Pane ( pane ) => {
                    pane.widget.try_lock().unwrap().process_data(&mut minicbor::Decoder::new(b"Hello, Zenoh 2!"));
                },
                _ => {}
            }
        }
    });
    tokio::spawn(async move {
        actor_zenoh.run().await;
    });
    info!("Data {:?}", static_data.data.try_lock().unwrap().value);
    static_data.data.try_lock().unwrap().value = "Hello, Zenoh 3!".to_string();

    eframe::run_simple_native("Zenoh ", options, move |ctx, _frame| {
        egui::CentralPanel::default().show(ctx, |ui| {
            ctx.set_visuals(Visuals::light());
            let mut behavior = TreeBehavior {};
            tree.ui(&mut behavior, ui);
        });
    })
}

fn create_tree() -> egui_tiles::Tree<Pane> {
    let mut next_view_nr = 0;
    let mut gen_pane = || {
        let pane = TextPane {
            text: format!("TextPane {}", next_view_nr),
        };
        next_view_nr += 1;
        Pane::new(pane)
    };

    let mut tiles = egui_tiles::Tiles::<Pane>::default();

    let mut tabs = vec![];
    tabs.push({
        let children = (0..7).map(|_| tiles.insert_pane(gen_pane())).collect();
        tiles.insert_horizontal_tile(children)
    });

    let root = tiles.insert_tab_tile(tabs);

    egui_tiles::Tree::new("my_tree", root, tiles)
}
