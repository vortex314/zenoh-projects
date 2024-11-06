cargo bloat --release --crates -n 20 --symbols-section .flash.text
# cargo bloat --release --crates -n 20 --symbols-section .iram0.text
cargo bloat --release -n 200 --symbols-section .flash.text | more
