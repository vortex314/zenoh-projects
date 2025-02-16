use std::{
    collections::VecDeque,
    fmt::{Display, Formatter},
    io::Cursor,
    sync::Arc,
    time::{Duration, Instant, SystemTime, UNIX_EPOCH},
};

use image::ImageReader;

use anyhow::Result;
use egui::{ColorImage, Context, Image, ImageSource};
use egui_plot::{Line, Plot, PlotPoints};
use egui_tiles::UiResponse;
use log::{debug, error, info};
use serde::{Deserialize, Serialize};

use crate::value::Value;

use super::{PaneWidget, PubSub};

#[derive(Debug, Serialize, Deserialize)]
enum Status {
    Ok,
    Warning,
    Error,
}

#[derive(Serialize, Deserialize)]
pub struct ImageWidget {
    #[serde(skip)]
    data: Option<Vec<u8>>,
    #[serde(skip)]
    texture: Option<egui::TextureHandle>,
    #[serde(skip)]
    ctx : Option<Context>,
    #[serde(skip)]
    last_update : Option<Instant>,
}

impl std::fmt::Debug for ImageWidget {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ImageWidget")
            .field("data", &self.data)
            .finish()
    }
}

impl ImageWidget {
    pub fn new() -> ImageWidget {
        ImageWidget {
            data: None,
            texture: None,
            ctx: None,
            last_update: None,
        }
    }
}

fn decode_bytes_to_texture(ctx: &egui::Context, data: &Vec<u8>) -> Result<egui::TextureHandle> {
    let start_time = Instant::now();
    let mut reader = ImageReader::new(Cursor::new(data));
    reader.set_format(image::ImageFormat::Jpeg);
    //       .with_guessed_format()?
    let image = reader.decode()?;
    debug!("ImageReader::new takes {:?}", start_time.elapsed());
    // Convert the image to RGBA
    let image_rgba = image.to_rgba8();
    debug!("image.to_rgba8 takes {:?}", start_time.elapsed());
    // Upload the image to the GPU as a texture
    let texture = ctx.load_texture(
        "my_texture",
        egui::ColorImage::from_rgba_unmultiplied(
            [image_rgba.width() as usize, image_rgba.height() as usize],
            &image_rgba,
        ),
        Default::default(),
    );
    info!("ctx.load_texture takes {:?}", start_time.elapsed());
    Ok(texture)
}

impl PaneWidget for ImageWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        let start_time = Instant::now();
        if self.ctx.is_none() {
            self.ctx = Some(ui.ctx().clone());
        }

        if self.texture.is_none() {
            return UiResponse::None;
        }

        if let Some(texture) = self.texture.as_ref() {
            // Display the image
            ui.image((texture.id(), ui.available_size()));
        } else {
            ui.label("Failed to decode image.");
        }
        let elapsed = start_time.elapsed();
        debug!("ImageWidget rendered in {:?}", elapsed);

        UiResponse::None
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.label("ImageWidget context menu");
    }

    fn process_data(&mut self, _topic: String, _value: &Value) {
        let too_soon = self.last_update.map(|last_update| {
             if last_update.elapsed() < Duration::from_secs(1) {
                debug!("ImageWidget received data too soon, dropping it");
                return true;
             } else   {
                 return false;
             }
        });
        if too_soon.unwrap_or(false) {
            return;
        }
        
        match _value {
            Value::Bytes(bytes) => {
                debug!("ImageWidget received image data [{}]", bytes.len());
                self.data = Some(bytes.clone());
                if let Some(ctx) = self.ctx.as_ref() {
                    let _  = decode_bytes_to_texture(ctx, bytes).map(|texture| {
                        self.texture = Some(texture);
                    });
                }
                self.last_update = Some(Instant::now());
            }
            _ => {
                error!("ImageWidget received non-image data");
            }
        }
    }
}
