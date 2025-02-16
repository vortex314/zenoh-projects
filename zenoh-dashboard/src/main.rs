#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::Arc;
use std::sync::Mutex;
use std::time::Duration;

use eframe::egui::{self, Ui, widget_text::WidgetText};
use egui_tiles::{Tile, TileId, Tiles};
use egui_extras::install_image_loaders;
mod pane;
use log::debug;
use log::error;
use pane::NullWidget;
use pane::Pane;
use pane::Widget;
mod value;
use pane::PaneWidget;
use pane::TextWidget;
use shared::on_shared;
use shared::update_with_value;
use shared::FieldInfo;
use value::Value;
mod actor_zenoh;
mod logger;
use actor_zenoh::{Actor, ZenohActor};
use log::info;
mod file_storage;

mod shared;
use shared::SHARED;

#[tokio::main(flavor = "multi_thread", worker_threads = 2)]

async fn main() -> Result<(), eframe::Error> {
    logger::init(); // Log to stderr (if you run with `RUST_LOG=debug`);

    let mut options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([800.0, 600.0]),
        ..Default::default()
    };
    options.persist_window = true;
    options.persistence_path = Some(PathBuf::from("./save.ron"));

    eframe::run_native(
        "Zenoh Dashboard",
        options,
        Box::new(|_cc| {
            #[cfg_attr(not(feature = "serde"), allow(unused_mut))]
            let mut app = MyApp::default();
            #[cfg(feature = "serde")]
            if let Some(storage) = _cc.storage {
                info!("Loading state");
                if let Some(state) = eframe::get_value(storage, eframe::APP_KEY) {
                    app = state;
                }
            }

            let tree_clone = app.tree.clone();
            let _registry_clone = app.registry.clone();
            let mut actor_zenoh: ZenohActor = ZenohActor::new();

            actor_zenoh.add_listener(move |_event| match _event {
                actor_zenoh::ZenohEvent::Publish { topic, payload } => {
                    let r = Value::from_cbor(payload.to_vec());
                    if let Ok(value) = r {
                        let s:String = value.to_string();
                        if  s.len() > 100 {
                            debug!(" RXD {} :{} ", topic, &s[0..100]);
                        } else {
                            debug!(" RXD {} :{} ", topic, s);
                        }
                        update_with_value(topic,&value);

                        let _ = tree_clone.lock().map(|mut tree_clone| {
                            for (_tile_id, tile_pane) in tree_clone.tiles.iter_mut() {
                                match tile_pane {
                                    egui_tiles::Tile::Pane(pane) => {
                                        pane.process_data(topic.clone(), &value);
                                    }
                                    egui_tiles::Tile::Container(_) => {}
                                }
                            }
                        });
                    } else {
                        error!("Error decoding payload from topic {} [{}] : {}", topic,payload.len(),r.err().unwrap());
                    }
                }
                _ => {}
            });

            tokio::spawn(async move {
                let r = actor_zenoh.run().await;
                if let Err(e) = r {
                    error!("Error running zenoh actor: {}", e);
                }
            });

            install_image_loaders(&_cc.egui_ctx);
            _cc.egui_ctx.set_theme(egui::Theme::Light); // Switch to light mode

            _cc.egui_ctx.style_mut(|style| {
                style.visuals.widgets.noninteractive.bg_fill = egui::Color32::from_rgb(0, 255, 0);
            });
            Ok(Box::new(app))
        }),
    )
}

fn registry_add_topic(topic: &String,value:&Value) {
    on_shared(|shared| {
        if shared.values.contains_key(topic) {
            return;
        }
        let mut fields = Vec::new();
        match value {
            Value::MapIdx(map) => {
                for (idx, (key, value)) in map.iter().enumerate() {
                    let field = FieldInfo {
                        idx,
                        name: key.to_string(),
                        desc: "".to_string(),
                        last_value: value.to_string(),
                    };
                    fields.push(field);
                }
            }
            Value::MapStr(map) => {
                for (idx, (key, value)) in map.iter().enumerate() {
                    let field = FieldInfo {
                        idx,
                        name: key.to_string(),
                        desc: "".to_string(),
                        last_value: value.to_string(),
                    };
                    fields.push(field);
                }
            }
            _ => {
                let field = FieldInfo {
                    idx: 0,
                    name: "".to_string(),
                    desc: "".to_string(),
                    last_value: value.to_string(),
                };
                fields.push(field);
            }
        }
       
    });
}


struct TreeBehavior {
    simplification_options: egui_tiles::SimplificationOptions,
    tab_bar_height: f32,
    gap_width: f32,
    add_child_to: Option<egui_tiles::TileId>,
}

impl Default for TreeBehavior {
    fn default() -> Self {
        Self {
            simplification_options: Default::default(),
            tab_bar_height: 24.0,
            gap_width: 1.0,
            add_child_to: None,
        }
    }
}

impl TreeBehavior {
    fn ui(&mut self, ui: &mut egui::Ui) {
        let Self {
            simplification_options,
            tab_bar_height,
            gap_width,
            add_child_to: _,
        } = self;
        /*
                Ui::style_mut(ui).visuals.widgets.inactive.bg_fill = Color32::BLUE;
                Ui::style_mut(ui).visuals.widgets.inactive.fg_stroke = Stroke::new(2.0,THEME.title_text_color);
                Ui::style_mut(ui).visuals.widgets.noninteractive.bg_fill = Color32::GREEN;
                Ui::style_mut(ui).visuals.widgets.noninteractive.fg_stroke = Stroke::new(2.0,THEME.title_text_color);
        */
        egui::Grid::new("behavior_ui")
            .num_columns(2)
            .show(ui, |ui| {
                ui.label("All panes must have tabs:");
                ui.checkbox(&mut simplification_options.all_panes_must_have_tabs, "");
                ui.end_row();

                ui.label("Join nested containers:");
                ui.checkbox(
                    &mut simplification_options.join_nested_linear_containers,
                    "",
                );
                ui.end_row();
                ui.label("Tab bar height:");
                ui.add(
                    egui::DragValue::new(tab_bar_height)
                        .range(0.0..=100.0)
                        .speed(1.0),
                );
                ui.end_row();
                ui.label("Gap width:");
                ui.add(egui::DragValue::new(gap_width).range(0.0..=20.0).speed(1.0));
                ui.end_row();
            });
    }
}

impl egui_tiles::Behavior<Pane> for TreeBehavior {
    fn pane_ui(
        &mut self,
        ui: &mut egui::Ui,
        _tile_id: egui_tiles::TileId,
        pane: &mut Pane,
    ) -> egui_tiles::UiResponse {
        pane.show(ui)
    }

    fn tab_title_for_pane(&mut self, pane: &Pane) -> egui::widget_text::WidgetText {
        format!("View {}", pane.title()).into()
    }

    fn top_bar_right_ui(
        &mut self,
        _tiles: &egui_tiles::Tiles<Pane>,
        ui: &mut egui::Ui,
        tile_id: egui_tiles::TileId,
        _tabs: &egui_tiles::Tabs,
        _scroll_offset: &mut f32,
    ) {
        if ui.button("➕").clicked() {
            self.add_child_to = Some(tile_id);
        }
        if ui.button("🔃").clicked() {
            info!("refreshing gui");
            // request_repaint();
        }
    }

    fn retain_pane(&mut self, _pane: &Pane) -> bool {
        _pane.retain()
    }

    // ---
    // Settings:

    fn tab_bar_height(&self, _style: &egui::style::Style) -> f32 {
        self.tab_bar_height
    }

    fn gap_width(&self, _style: &egui::style::Style) -> f32 {
        self.gap_width
    }

    fn simplification_options(&self) -> egui_tiles::SimplificationOptions {
        self.simplification_options
    }

    fn is_tab_closable(&self, _tiles: &Tiles<Pane>, _tile_id: TileId) -> bool {
        true
    }

    fn on_edit(&mut self, _edit_action: egui_tiles::EditAction) {
        // log::info!("Edit: {:?}", edit_action);
    }

    fn on_tab_close(&mut self, tiles: &mut Tiles<Pane>, tile_id: TileId) -> bool {
        if let Some(tile) = tiles.get(tile_id) {
            match tile {
                Tile::Pane(pane) => {
                    // Single pane removal
                    let tab_title = self.tab_title_for_pane(pane);
                    log::info!("Closing tab: {}, tile ID: {tile_id:?}", tab_title.text());
                }
                Tile::Container(container) => {
                    // Container removal
                    log::info!("Closing container: {:?}", container.kind());
                    let children_ids = container.children();
                    for child_id in children_ids {
                        if let Some(Tile::Pane(pane)) = tiles.get(*child_id) {
                            let tab_title = self.tab_title_for_pane(pane);
                            log::info!("Closing tab: {}, tile ID: {tile_id:?}", tab_title.text());
                        }
                    }
                }
            }
        }

        // Proceed to removing the tab
        true
    }
}

#[cfg_attr(feature = "serde", derive(serde::Deserialize, serde::Serialize))]
struct MyApp {
    tree: Arc<Mutex<egui_tiles::Tree<Pane>>>,
    #[cfg_attr(feature = "serde", serde(skip))]
    behavior: TreeBehavior,
    #[cfg_attr(feature = "serde", serde(skip))]
    registry: Arc<Mutex<HashMap<String, Vec<FieldInfo>>>>,
}

impl Default for MyApp {
    fn default() -> Self {
        let mut next_view_nr = 0;
        let mut gen_view = || {
            let mut view = Pane::new(Widget::TextWidget(TextWidget::new()));
            view.title = format!("View {}", next_view_nr);
            next_view_nr += 1;
            view
        };

        let mut tiles = egui_tiles::Tiles::default();

        let mut tabs = vec![];
        let tab_tile = {
            let children = (0..7).map(|_| tiles.insert_pane(gen_view())).collect();
            tiles.insert_tab_tile(children)
        };
        tabs.push(tab_tile);
        tabs.push({
            let children = (0..7).map(|_| tiles.insert_pane(gen_view())).collect();
            tiles.insert_horizontal_tile(children)
        });
        tabs.push({
            let children = (0..7).map(|_| tiles.insert_pane(gen_view())).collect();
            tiles.insert_vertical_tile(children)
        });
        tabs.push({
            let cells = (0..11).map(|_| tiles.insert_pane(gen_view())).collect();
            tiles.insert_grid_tile(cells)
        });
        tabs.push(tiles.insert_pane(gen_view()));

        let root = tiles.insert_tab_tile(tabs);

        let tree = Arc::new(Mutex::new(egui_tiles::Tree::new("my_tree", root, tiles)));

        let registry = Arc::new(Mutex::new(HashMap::new()));

        Self {
            tree,
            behavior: Default::default(),
            registry,
        }
    }
}

impl eframe::App for MyApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            let _ = self
                .tree
                .lock()
                .map(|mut tree| tree.ui(&mut self.behavior, ui));
        });
        // refresh the gui every 100ms
        ctx.request_repaint_after(Duration::from_millis(100));
        // adding a child to the selected tab
        if let Some(parent) = self.behavior.add_child_to.take() {
            let _ = self.tree.lock().map(|mut tree| {
                let new_child = tree
                    .tiles
                    .insert_pane(Pane::new(Widget::NullWidget(NullWidget::new())));
                if let Some(egui_tiles::Tile::Container(egui_tiles::Container::Tabs(tabs))) =
                    tree.tiles.get_mut(parent)
                {
                    tabs.add_child(new_child);
                    tabs.set_active(new_child);
                }
            });
        }
    }

    fn save(&mut self, _storage: &mut dyn eframe::Storage) {
        info!("Saving state");
        #[cfg(feature = "serde")]
        eframe::set_value(_storage, eframe::APP_KEY, &self);
    }
}
