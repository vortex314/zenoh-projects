use anyhow::Result;
use crossterm::{
    event::{self, Event, KeyCode},
    execute,
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
};
use dashmap::DashMap;
use limeros::{
    Endpoint, UdpMessage, UdpMessageHandler, UdpNode, msgs::{HoverboardCmd, HoverboardEvent, SysEvent, TypedMessage, WifiEvent}
}; // Assuming the code you provided is in the same crate
use log::info;
use ratatui::{
    backend::{Backend, CrosstermBackend},
    layout::{Constraint, Direction, Layout, Rect},
    widgets::{Block, Borders, Cell, Paragraph, Row, Table, Tabs},
    Frame, Terminal,
};
use std::{
    io,
    sync::Arc,
    time::{Duration, SystemTime, Instant},
};
use tokio::sync::Mutex;

// --- 1. App State ---

#[derive(Clone)]
struct Record {
    pub timestamp: SystemTime,
    pub src: String,
    pub msg_type: String,
    pub field_name: String,
    pub value: String,
}

impl PartialEq for Record {
    fn eq(&self, other: &Self) -> bool {
        self.src == other.src
            && self.msg_type == other.msg_type
            && self.field_name == other.field_name
    }
}

impl Eq for Record {}

impl std::hash::Hash for Record {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.src.hash(state);
        self.msg_type.hash(state);
        self.field_name.hash(state);
    }
}

#[derive(Clone, Copy, PartialEq)]
enum SortColumn {
    Time,
    Source,
    Type,
    Field,
}

struct App {
    node: Arc<UdpNode>,
    titles: Vec<&'static str>,
    index: usize,
    // Storage for the "Events" tab
    cache: Arc<DashMap<String, Record>>,
    sort_column: SortColumn,
    sort_desc: bool,
    // Cached endpoints data
    endpoints: Vec<Endpoint>, // (id, address, ms_ago)
}

impl App {
    fn new(node: Arc<UdpNode>) -> Self {
        Self {
            node,
            titles: vec!["Endpoints", "Live Events"],
            index: 0,
            cache: Arc::new(DashMap::new()),
            sort_column: SortColumn::Time,
            sort_desc: true,
            endpoints: Vec::new(),
        }
    }

    pub fn next(&mut self) {
        self.index = (self.index + 1) % self.titles.len();
    }

    pub fn cycle_sort(&mut self, col: SortColumn) {
        if self.sort_column == col {
            self.sort_desc = !self.sort_desc;
        } else {
            self.sort_column = col;
            self.sort_desc = false;
        }
    }
}

// --- 2. Generic Handler for TUI Update ---

struct TuiHandler {
    state: Arc<Mutex<App>>,
}

#[async_trait::async_trait]
impl UdpMessageHandler for TuiHandler {
    async fn handle(&self, udp_message: &UdpMessage) -> anyhow::Result<()> {
        let mut app = self.state.lock().await;

        // Parse the payload (assuming JSON as per your UdpNode implementation)
        let fields = if let Some(payload) = &udp_message.payload {
            let v: serde_json::Value = serde_json::from_slice(payload).unwrap_or_default();
            if let serde_json::Value::Object(map) = v {
                map.into_iter().map(|(k, v)| (k, v.to_string())).collect()
            } else {
                vec![(
                    "raw".to_string(),
                    String::from_utf8_lossy(payload).into_owned(),
                )]
            }
        } else {
            vec![]
        };

        for (field_name, value) in &fields {
            let key = format!(
                "{}:{}:{}",
                udp_message.src.clone().unwrap_or_default(),
                udp_message.msg_type.clone().unwrap_or_default(),
                field_name
            );
            let record = Record {
                timestamp: SystemTime::now(),
                src: udp_message.src.clone().unwrap_or_default(),
                msg_type: udp_message.msg_type.clone().unwrap_or_default(),
                field_name: field_name.clone(),
                value: value.clone(),
            };
            app.cache
                .entry(key)
                .and_modify(|e| {
                    *e = record.clone();
                })
                .or_insert(record);
        }

        Ok(())
    }
}

// --- 3. Main UI Loop ---

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    // Setup Terminal
    enable_raw_mode()?;
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen)?;
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;

    // Initialize UdpNode
    let node = UdpNode::new("tui-monitor", "239.0.0.1:50000").await?;
    node.add_subscription(SysEvent::MSG_TYPE).await;
    node.add_subscription(WifiEvent::MSG_TYPE).await;
    node.add_subscription(HoverboardEvent::MSG_TYPE).await;
    let app_state = Arc::new(Mutex::new(App::new(node.clone())));

    // Register our TUI handler to the node
    node.add_generic_handler(TuiHandler {
        state: app_state.clone(),
    })
    .await;

    // Run TUI loop
    let res = run_app(&mut terminal, app_state).await;

    // Restore Terminal
    disable_raw_mode()?;
    execute!(terminal.backend_mut(), LeaveAlternateScreen)?;
    terminal.show_cursor()?;

    if let Err(err) = res {
        println!("{:?}", err)
    }

    Ok(())
}

async fn run_app<B: Backend>(terminal: &mut Terminal<B>, app: Arc<Mutex<App>>) -> Result<()> {
    let mut last_update = Instant::now();

    loop {
        // Update endpoints data periodically (every 200ms)
        if last_update.elapsed() > Duration::from_millis(200) {
            let mut app_w = app.lock().await;
            let endpoints_data = app_w.node.get_endpoints().await;
            app_w.endpoints = endpoints_data
                .iter()
                .map(|e| e.value().clone())
                .collect();
            drop(app_w); // Release lock immediately
            last_update = Instant::now();
        }

        // Try to lock without blocking indefinitely
        if let Ok(app_snapshot) = app.try_lock() {
            let endpoints = app_snapshot.endpoints.clone();
            let cache = app_snapshot.cache.clone();
            let index = app_snapshot.index;
            let titles = app_snapshot.titles.clone();
            let sort_column = app_snapshot.sort_column;
            let sort_desc = app_snapshot.sort_desc;
            drop(app_snapshot); // Release lock immediately

            terminal
                .draw(|f| ui(f, &titles, index, cache, &endpoints,sort_column, sort_desc))
                .map_err(|e| anyhow::anyhow!("Draw error: {}", e))?;
        }

        if event::poll(Duration::from_millis(50))? {
            if let Event::Key(key) = event::read()? {
                let mut app_w = app.lock().await;
                match key.code {
                    KeyCode::Char('q') => return Ok(()),
                    KeyCode::Tab => app_w.next(),
                    // Sorting keys
                    KeyCode::Char('1') => app_w.cycle_sort(SortColumn::Time),
                    KeyCode::Char('2') => app_w.cycle_sort(SortColumn::Source),
                    KeyCode::Char('3') => app_w.cycle_sort(SortColumn::Type),
                    KeyCode::Char('4') => app_w.cycle_sort(SortColumn::Field),
                    _ => {}
                }
            }
        }
    }
}

// --- 4. UI Rendering ---

fn ui(
    f: &mut Frame,
    titles: &[&'static str],
    index: usize,
    cache: Arc<DashMap<String, Record>>,
    endpoints: &Vec<Endpoint>,
    sort_column: SortColumn,
    sort_desc: bool,
) {
    let size = f.area();
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([Constraint::Length(3), Constraint::Min(0)].as_ref())
        .split(size);

    let titles_vec: Vec<_> = titles
        .iter()
        .cloned()
        .map(ratatui::text::Line::from)
        .collect();
    let tabs = Tabs::new(titles_vec)
        .block(
            Block::default()
                .borders(Borders::ALL)
                .title("UDP Node Monitor (Tab to switch, Q to quit)"),
        )
        .select(index)
        .highlight_style(ratatui::style::Style::default().fg(ratatui::style::Color::Yellow));
    f.render_widget(tabs, chunks[0]);

    match index {
        0 => render_endpoints_tab(f, chunks[1], endpoints),
        1 => render_events_tab(f, chunks[1], cache, sort_column, sort_desc),
        _ => {}
    };
}

fn render_endpoints_tab(f: &mut Frame, area: Rect, endpoints: &Vec<Endpoint>) {
    let rows: Vec<Row> = endpoints
        .iter()
        .map(|ep| {
            Row::new(vec![
                Cell::from(ep.name.clone()),
                Cell::from(ep.addr.to_string()),
                Cell::from(format!("{:?}", ep.subscribe)),
                Cell::from(format!("{} sec ago", Instant::now().duration_since(ep.last_seen).as_secs())),
            ])
        })
        .collect();

    let table = Table::new(
        rows,
        [
            Constraint::Percentage(20),
            Constraint::Percentage(20),
            Constraint::Percentage(40),
            Constraint::Percentage(20),
        ],
    )
    .header(
        Row::new(vec!["Peer ID", "Address", "Last Seen"])
            .style(ratatui::style::Style::default().fg(ratatui::style::Color::Cyan)),
    )
    .block(
        Block::default()
            .borders(Borders::ALL)
            .title("Network Discovery"),
    );

    f.render_widget(table, area);
}

fn system_time_to_hhmmss(system_time: &SystemTime) -> String {
    let datetime: chrono::DateTime<chrono::Local> = (*system_time).into();
    datetime.format("%H:%M:%S").to_string()
}

fn render_events_tab(f: &mut Frame, area: Rect, cache: Arc<DashMap<String, Record>>, sort_column: SortColumn, sort_desc: bool) {
    let mut rows = Vec::new();

    // Collect records into a Vec first, then reverse iterate
    let mut records: Vec<Record> = Vec::new();
    for entry in cache.iter() {
        records.push(entry.value().clone());
    }

    // 2. Sort Records
    records.sort_by(|a, b| {
        let ord = match sort_column {
            SortColumn::Time => a.timestamp.cmp(&b.timestamp),
            SortColumn::Source => a.src.cmp(&b.src),
            SortColumn::Type => a.msg_type.cmp(&b.msg_type),
            SortColumn::Field => a.field_name.cmp(&b.field_name),
        };
        if sort_desc {
            ord.reverse()
        } else {
            ord
        }
    });
    // Flattening captured messages into one record per field as requested
    for record in records.iter().rev() {
        rows.push(Row::new(vec![
            Cell::from(system_time_to_hhmmss(&record.timestamp)),
            Cell::from(record.src.clone()),
            Cell::from(record.msg_type.clone()),
            Cell::from(record.field_name.clone()),
            Cell::from(record.value.clone()),
        ]));
    }

    // 3. Build Header with Indicators
    let header_titles = [
        (SortColumn::Time, "Time (1)"),
        (SortColumn::Source, "Source (2)"),
        (SortColumn::Type, "Type (3)"),
        (SortColumn::Field, "Field (4)"),
    ];


    let header_cells = header_titles.iter().map(|(col, title)| {
        let mut text = title.to_string();
        if sort_column == *col {
            text += if sort_desc { " ▼" } else { " ▲" };
        }
        Cell::from(text).style(ratatui::style::Style::default().fg(ratatui::style::Color::Cyan))
    }).chain(std::iter::once(Cell::from("Value")));


    let table = Table::new(
        rows,   
        [
            Constraint::Length(10),
            Constraint::Length(15),
            Constraint::Length(15),
            Constraint::Length(15),
            Constraint::Min(20),
        ],
    )
    .header(
        Row::new(header_cells)
            .style(ratatui::style::Style::default().fg(ratatui::style::Color::Cyan)),
    )
    .block(
        Block::default()
            .borders(Borders::ALL)
            .title("Incoming Typed Messages"),
    );

    f.render_widget(table, area);
}
