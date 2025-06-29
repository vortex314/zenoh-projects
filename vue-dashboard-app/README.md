# Vue Dashboard Application

## Overview
This project is a dynamic dashboard application built with Vue.js that utilizes Gridstack for layout management, ECharts for data visualization, and WebSockets for real-time server communication. The dashboard allows users to add, resize, and drag widgets, which can display charts and gauges that receive continuous updates via WebSockets.

## Features
- **Dynamic Dashboard**: Users can add, remove, and rearrange widgets on the dashboard.
- **Real-time Updates**: Widgets subscribe to WebSocket topics to receive live data updates.
- **Chart and Gauge Widgets**: Includes specialized widgets for displaying charts and gauges using ECharts.
- **Layout Persistence**: Users can save and load their dashboard layout, preserving widget arrangements.

## Project Structure
```
vue-dashboard-app
├── public
│   └── index.html
├── src
│   ├── assets
│   ├── components
│   │   ├── Dashboard.vue
│   │   ├── Widget.vue
│   │   ├── ChartWidget.vue
│   │   └── GaugeWidget.vue
│   ├── services
│   │   ├── websocket.js
│   │   └── layout.js
│   ├── utils
│   │   └── topics.js
│   ├── App.vue
│   ├── main.js
│   └── styles
│       └── gridstack.css
├── package.json
├── vite.config.js
└── README.md
```

## Installation
1. Clone the repository:
   ```
   git clone <repository-url>
   cd vue-dashboard-app
   ```

2. Install dependencies:
   ```
   npm install
   ```

3. Start the development server:
   ```
   npm run dev
   ```

## Usage
- Open your browser and navigate to `http://localhost:3000` (or the port specified in your Vite configuration).
- Use the dashboard interface to add and arrange widgets.
- Widgets will automatically update with real-time data from the server.

## Technologies Used
- **Vue.js**: JavaScript framework for building user interfaces.
- **Gridstack**: Library for creating draggable and resizable grid layouts.
- **ECharts**: Powerful charting and visualization library.
- **WebSockets**: For real-time communication with the server.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request for any enhancements or bug fixes.

## License
This project is licensed under the MIT License. See the LICENSE file for details.