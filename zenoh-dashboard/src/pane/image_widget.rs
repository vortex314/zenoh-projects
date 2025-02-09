use std::{
    collections::VecDeque,
    time::{Duration, SystemTime, UNIX_EPOCH},
    io::Cursor,
};

use image::io::Reader as ImageReader;

use egui::{ColorImage, Image};
use egui_plot::{Line, Plot, PlotPoints};
use egui_tiles::UiResponse;
use log::info;
use serde::{Deserialize, Serialize};

use crate::value::Value;

use super::{PaneWidget, PubSub};

#[derive(Debug, Serialize, Deserialize)]
enum Status {
    Ok,
    Warning,
    Error,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ImageWidget {
    #[serde(skip)]
    data: Option<Vec<u8>>,
}

impl ImageWidget {
    pub fn new() -> ImageWidget {
        ImageWidget { data: None }
    }

    fn decode_jpeg_to_texture(&self, ctx: &egui::Context) -> Option<egui::TextureHandle> {
        // Get the JPEG binary data
        let jpeg_data = self.data.as_ref()?;

        // Decode the JPEG using the `image` crate
        let img = match ImageReader::new(Cursor::new(jpeg_data))
            .with_guessed_format()
            .unwrap()
            .decode()
        {
            Ok(img) => img,
            Err(e) => {
                eprintln!("Failed to decode JPEG: {}", e);
                return None;
            }
        };

        // Convert the image to RGB pixels
        let img_rgb = img.to_rgb8();

        // Create an egui ColorImage
        let size = [img_rgb.width() as usize, img_rgb.height() as usize];
        let pixels = img_rgb.to_vec();
        let color_image = ColorImage::from_rgb(size, &pixels);

        // Create and return an egui texture
        Some(ctx.load_texture("camera_image", color_image,Default::default()))
    }
}

impl PaneWidget for ImageWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        if self.data.is_none() {
            return UiResponse::None;
        }
        // Decode the JPEG data into an egui texture
        let texture = self.decode_jpeg_to_texture(ui.ctx());
        if let Some(texture) = texture {
            // Display the image
            ui.add(Image::new(&texture).max_width(400.0));
        } else {
            ui.label("Failed to decode image.");
        }

        UiResponse::None
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.label("ImageWidget context menu");
    }

    fn process_data(&mut self, _topic: String, _value: &Value) {
        match _value {
            Value::Bytes(bytes) => {
                self.data = Some(bytes.clone());
            }
            _ => {
                info!("ImageWidget received non-image data");
            }
        }
    }
}
