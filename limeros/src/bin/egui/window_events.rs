use std::{collections::HashSet, sync::Arc};

use anyhow::Result;
use dashmap::DashMap;
use eframe::egui;
use limeros::UdpMessage;
use tokio::sync::Mutex;

use crate::{Record, my_window::{ MyWindow}};

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
    window_others: Arc<DashMap<String, Box<dyn MyWindow>>>,
    graph_data: Arc<DashMap<String, crate::MetricData>>,
}

impl WindowEvents {
    pub fn new(
        cache: Arc<DashMap<String, Record>>,
        window_others: Arc<DashMap<String, Box<dyn MyWindow>>>,
        graph_data: Arc<DashMap<String, crate::MetricData>>,
    ) -> Self {
        Self {
            cache,
            sort_col: SortColumn::Field,
            sort_desc: true,
            selected_fields: HashSet::new(),
            window_others,
            graph_data,
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
                                    let mut keys = Vec::new();

                                    if self.selected_fields.len() == 0 {
                                        keys.push(key.clone());
                                    } else {
                                        for k in &self.selected_fields {
                                            keys.push(k.clone());
                                        }
                                    }
                                    let window_key = format!("{}_plot", key);
                                    if !self.window_others.contains_key(&window_key) {
                                        self.window_others.insert(
                                            window_key.clone(),
                                            Box::new(crate::window_plot::WindowPlot::new(
                                                keys,
                                                self.graph_data.clone(),
                                            )),
                                        );
                                    }
                                }
                                if ui.button("⭕").clicked() {
                                    let window_key = format!("{}_gauge", key);
                                    if !self.window_others.contains_key(&window_key) {
                                        self.window_others.insert(
                                            key.clone(),
                                            Box::new(crate::window_gauge::WindowGauge::new(
                                                key.clone(),
                                                self.cache.clone(),
                                            )),
                                        );
                                    }
                                }
                                if ui.button("|").clicked() {}
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

    fn show(&mut self, ui: &mut egui::Ui) -> Result<()> {
        egui::Window::new("Events")
            .open(&mut true)
            .resizable([true, true])
            .constrain_to(ui.available_rect_before_wrap())
            .show(ui.ctx(), |ui| {
                self.render_events(ui);
            });
        Ok(())
    }
    fn is_closed(&self) -> bool {
        false
    }
}
