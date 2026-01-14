use std::{collections::HashSet, sync::Arc};

use dashmap::DashMap;
use eframe::egui;
use limeros::UdpMessage;

use crate::{Record, my_window::MyWindow};

#[derive(PartialEq, Clone, Copy)]
enum SortColumn {
    Time,
    Source,
    Type,
    Field,
}

pub struct WindowEvents {
    cache: Arc<DashMap<String, Record>>,
    sort_col: SortColumn,
    sort_desc: bool,
    selected_fields: HashSet<String>,
    widget_windows: DashMap<String, HashSet<String>>,
}

impl WindowEvents {
    pub fn new(cache: Arc<DashMap<String, Record>>) -> Self {
        Self {
            cache,
            sort_col: SortColumn::Time,
            sort_desc: true,
            selected_fields: HashSet::new(),
            widget_windows: DashMap::new(),
        }
    }       
    fn toggle_sort(&mut self, col: SortColumn) {
        if self.sort_col == col {
            self.sort_desc = !self.sort_desc;
        } else {
            self.sort_col = col;
            self.sort_desc = false;
        }
    }


    fn render_events(&mut self, ui: &mut egui::Ui) {
        egui::ScrollArea::vertical().show(ui, |ui| {
            use egui_extras::{Column, TableBuilder};

            let mut records: Vec<Record> = self.cache.iter().map(|e| e.value().clone()).collect();

            // Sort logic
            records.sort_by(|a, b| {
                let ord = match self.sort_col {
                    SortColumn::Time => a.timestamp.cmp(&b.timestamp),
                    SortColumn::Source => a.src.cmp(&b.src),
                    SortColumn::Type => a.msg_type.cmp(&b.msg_type),
                    SortColumn::Field => a.field_name.cmp(&b.field_name),
                };
                if self.sort_desc {
                    ord.reverse()
                } else {
                    ord
                }
            });

            TableBuilder::new(ui)
                .striped(true)
                .column(Column::initial(80.0))
                .column(Column::initial(80.0))
                .column(Column::initial(80.0))
                .column(Column::initial(80.0))
                .column(Column::initial(80.0))
                .column(Column::initial(200.0))
                .header(20.0, |mut header| {
                    header.col(|ui| {
                        ui.label("Time");
                    });
                    header.col(|ui| {
                        if ui.label("Source").clicked() {
                            self.toggle_sort(SortColumn::Source);
                        }
                    });
                    header.col(|ui| {
                        if ui.label("Type").clicked() {
                            self.toggle_sort(SortColumn::Type);
                        }
                    });
                    header.col(|ui| {
                        if ui.label("Field").clicked() {
                            self.toggle_sort(SortColumn::Field);
                        }
                    });
                    header.col(|ui| {
                        ui.label("Value");
                    });
                    header.col(|ui| {
                        ui.label("Functions");
                    });
                })
                .body(|body| {
                    body.rows(20.0, records.len(), |mut row| {
                        let r = &records[row.index()];
                        let time_str = chrono::DateTime::<chrono::Local>::from(r.timestamp)
                            .format("%H:%M:%S")
                            .to_string();
                        let key = format!("{}:{}:{}", r.src, r.msg_type, r.field_name);

                        row.col(|ui| {
                            ui.label(time_str);
                        });
                        row.col(|ui| {
                            ui.label(&r.src);
                        });
                        row.col(|ui| {
                            ui.label(&r.msg_type);
                        });
                        row.col(|ui| {
                            ui.label(&r.field_name);
                        });

                        row.col(|ui| {
                            ui.horizontal(|ui| {
                                let mut selected = self.selected_fields.contains(key.as_str());
                                if ui.checkbox(&mut selected, "").changed() {
                                    if selected {
                                        self.selected_fields.insert(key.clone());
                                    } else {
                                        self.selected_fields.remove(&key);
                                    }
                                    }
                                
                                if ui.button("📈").clicked() {
                                    // Could implement functionality to highlight/select this field for graphing
                                    self.widget_windows.entry("plot".to_string()).or_default().insert(key.clone());
                                }
                                if ui.button("⭕").clicked() {
                                    self.widget_windows.entry("gauge".to_string()).or_default().insert(key.clone());
                                }
                                if ui.button("|").clicked() {
                                    self.widget_windows.entry("bar".to_string()).or_default().insert(key.clone());
                                }
                                if ui.button("❌").clicked() {
                                    self.widget_windows.entry("plot".to_string()).or_default().remove(&key);
                                    self.widget_windows.entry("gauge".to_string()).or_default().remove(&key);
                                    self.widget_windows.entry("bar".to_string()).or_default().remove(&key);
                                }
                            });
                        });
                        row.col(|ui| {
                            ui.label(&r.value);
                        });
                    });
                });
        });
    }
}   

impl MyWindow for WindowEvents {
    fn name(&self) -> &'static str {
        "Events"
    }

    fn show(&mut self, ui: &mut egui::Ui) {
        egui::Window::new("Events")
            .open(&mut true)
            .resizable([true, true])
            .constrain_to(ui.available_rect_before_wrap())
            .show(ui.ctx(), |ui| {
                self.render_events(ui);
            });
    }

    fn ui(&mut self, ui: &mut egui::Ui) {
        self.render_events(ui);
    }

    fn on_message(&mut self, _udp_message: &UdpMessage) {
        // No message handling needed for this window
    }
}