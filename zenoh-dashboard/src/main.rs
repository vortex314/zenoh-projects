#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release

use serde::{Deserialize, Serialize};

mod actor_zenoh;

trait PaneWidget: std::fmt::Debug {
    fn ui(&mut self, ui: &mut egui::Ui);
    fn title(&self) -> String;
}

#[derive(Debug)]
struct Pane {
    widget: Box<dyn PaneWidget>,
}

impl Pane {
    fn new(widget: impl PaneWidget + 'static) -> Self {
        Self {
            widget: Box::new(widget),
        }
    }
}

#[derive(Debug)]
struct TextPane {
    text: String,
}

impl PaneWidget for TextPane {
    fn ui(&mut self, ui: &mut egui::Ui) {
        ui.label(&self.text);
    }

    fn title(&self) -> String {
        self.text.clone()
    }
}

struct TreeBehavior {}

impl egui_tiles::Behavior<Pane> for TreeBehavior {
    fn tab_title_for_pane(&mut self, pane: &Pane) -> egui::WidgetText {
        pane.widget.title().into()
        // format!("Pane {}", pane.nr).into()
    }

    fn pane_ui(
        &mut self,
        ui: &mut egui::Ui,
        _tile_id: egui_tiles::TileId,
        pane: &mut Pane,
    ) -> egui_tiles::UiResponse {
        let response = if ui
            .add(egui::Button::new("Hello, world!").sense(egui::Sense::drag()))
            .drag_started()
        {
            egui_tiles::UiResponse::DragStarted
        } else {
            egui_tiles::UiResponse::None
        };
        pane.widget.ui(ui);
        // Give each pane a unique color:
        /*  let color = egui::epaint::Hsva::new(0.103 * pane.nr as f32, 0.5, 0.5, 1.0);
        ui.painter().rect_filled(ui.max_rect(), 0.0, color);

        ui.label(format!("The contents of pane {}.", pane.nr));

        // You can make your pane draggable like so:
        if ui
            .add(egui::Button::new("Drag me!").sense(egui::Sense::drag()))
            .drag_started()
        {
            egui_tiles::UiResponse::DragStarted
        } else {
            egui_tiles::UiResponse::None
        }*/
        response
    }
}

fn main() -> Result<(), eframe::Error> {
    env_logger::init(); // Log to stderr (if you run with `RUST_LOG=debug`).

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([640.0, 480.0]),
        ..Default::default()
    };

    let mut tree = create_tree();

    eframe::run_simple_native("My egui App", options, move |ctx, _frame| {
        egui::CentralPanel::default().show(ctx, |ui| {
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
    tabs.push({
        let cells = (0..11).map(|_| tiles.insert_pane(gen_pane())).collect();
        let _ = tiles.insert_pane(Pane::new(TextPane {
            text: "Hello, world!".to_owned(),
        }));
        tiles.insert_grid_tile(cells)
    });
    tabs.push(tiles.insert_pane(gen_pane()));

    let root = tiles.insert_tab_tile(tabs);

    egui_tiles::Tree::new("my_tree", root, tiles)
}
