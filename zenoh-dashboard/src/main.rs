#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use egui::Visuals;
// hide console window on Windows in release
#[allow(unused_imports)]
use serde::{Deserialize, Serialize};

mod actor_zenoh;
use actor_zenoh::{Actor, ActorZenoh};
mod logger;
use log::info;
use logger::init;
use tokio::sync::Mutex;
trait PaneWidget: std::fmt::Debug {
    fn show(&mut self, ui: &mut egui::Ui);
    fn title(&self) -> String;
    fn on_message(&mut self, message: String) {}
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
    }

    fn title(&self) -> String {
        self.text.clone()
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
        ui.add(egui::widgets::Label::new(format!(
            "Rect [{},{}] , [{},{}] ",
            rect.left(),
            rect.top(),
            rect.right(),
            rect.bottom(),
        )));
        let response = if ui
            .add(egui::Button::new("Hello, world!").sense(egui::Sense::drag()))
            .drag_started()
        {
            egui_tiles::UiResponse::DragStarted
        } else {
            egui_tiles::UiResponse::None
        };
        pane.widget.try_lock().unwrap().show(ui);
        response
    }
}

#[tokio::main(flavor = "multi_thread", worker_threads = 2)]
async fn main() -> Result<(), eframe::Error> {
    logger::init();
    info!("Starting dashboard ");

    let mut actor_zenoh = ActorZenoh::new();
    tokio::spawn(async move {
        actor_zenoh.run().await;
    });

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([640.0, 480.0]),
        ..Default::default()
    };

    let mut tree = create_tree();

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
            text: format!("{}", next_view_nr),
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
