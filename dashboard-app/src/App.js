import React, { useState, useEffect, useRef } from 'react';
import { GridStack } from 'gridstack';
import 'gridstack/dist/gridstack.min.css';
import ReactECharts from 'echarts-for-react';
import io from 'socket.io-client';

// Widget types configuration
const WIDGET_TYPES = {
  LINE_CHART: {
    name: 'Line Chart',
    create: (id, topic, initialData) => ({
      id,
      type: 'LINE_CHART',
      topic,
      title: `Line Chart - ${topic}`,
      x: 0,
      y: 0,
      w: 4,
      h: 3,
      data: initialData || [],
      options: {
        title: { text: `Line Chart - ${topic}` },
        tooltip: {},
        xAxis: { type: 'category', data: initialData?.map((_, i) => i) || [] },
        yAxis: { type: 'value' },
        series: [{ data: initialData || [], type: 'line' }]
      }
    })
  },

  BAR_CHART: {
    name: 'Bar Chart',
    create: (id, topic, initialData) => ({
      id,
      type: 'BAR_CHART',
      topic,
      title: `Bar Chart - ${topic}`,
      x: 0,
      y: 0,
      w: 4,
      h: 3,
      data: initialData || [],
      options: {
        title: { text: `Bar Chart - ${topic}` },
        tooltip: {},
        xAxis: { type: 'category', data: initialData?.map((_, i) => `Cat ${i}`) || [] },
        yAxis: { type: 'value' },
        series: [{ data: initialData || [], type: 'bar' }]
      }
    })
  },
  PIE_CHART: {
    name: 'Pie Chart',
    create: (id, topic, initialData) => ({
      id,
      type: 'PIE_CHART',
      topic,
      title: `Pie Chart - ${topic}`,
      x: 0,
      y: 0,
      w: 3,
      h: 3,
      data: initialData || [10, 20, 30, 40, 50],
      options: {
        title: { text: `Pie Chart - ${topic}` },
        tooltip: { trigger: 'item' },
        series: [{
          type: 'pie',
          radius: '70%',
          data: (initialData || [10, 20, 30, 40, 50]).map((value, i) => ({
            value,
            name: `Item ${i}`
          }))
        }]
      }
    })
  },
  GAUGE: {
    name: 'Gauge',
    create: (id, topic, initialData) => ({
      id,
      type: 'GAUGE',
      topic,
      title: `Gauge - ${topic}`,
      x: 0,
      y: 0,
      w: 3,
      h: 3,
      data: initialData || 50,
      options: {
        title: { text: `Gauge - ${topic}` },
        tooltip: { formatter: '{a} <br/>{b} : {c}%' },
        series: [{
          name: 'Pressure',
          type: 'gauge',
          progress: { show: true },
          detail: { valueAnimation: true, formatter: '{value}' },
          data: [{ value: initialData || 50, name: topic }],
          axisLine: { lineStyle: { width: 10 } },
          axisTick: { distance: -30, length: 8, lineStyle: { color: '#999', width: 2 } },
          splitLine: { distance: -30, length: 30, lineStyle: { color: '#999', width: 4 } },
          axisLabel: { distance: 25, color: '#999', fontSize: 12 },
          pointer: { show: true }
        }]
      }
    })
  }
};

const Dashboard = () => {
  const [widgets, setWidgets] = useState([]);
  const [availableTopics, setAvailableTopics] = useState(['temperature', 'humidity', 'pressure', 'sales']);
  const [selectedTopic, setSelectedTopic] = useState('');
  const [selectedWidgetType, setSelectedWidgetType] = useState('LINE_CHART');
  const [savedLayouts, setSavedLayouts] = useState([]);
  const [layoutName, setLayoutName] = useState('');
  const gridRef = useRef(null);
  const socketRef = useRef(null);

  // Initialize WebSocket connection
useEffect(() => {
  // Create socket connection
  const socket = io('http://localhost:3001', {
    autoConnect: true,
    reconnection: true,
  });

  socketRef.current = socket;

  // Handle connection errors
  socket.on('connect_error', (err) => {
    console.error('WebSocket connection error:', err);
  });

  socket.on('connect', () => {
    console.log('WebSocket connected');
  });

  socket.on('disconnect', () => {
    console.log('WebSocket disconnected');
  });

  // Only add listener if socket is connected
  socket.on('data', (data) => {
    setWidgets(prevWidgets => 
      prevWidgets.map(widget => 
        widget.topic === data.topic 
          ? updateWidgetData(widget, data.value) 
          : widget
      )
    );
  });

  // Load saved layouts from localStorage
  const layouts = JSON.parse(localStorage.getItem('dashboardLayouts')) || [];
  setSavedLayouts(layouts);

  // Clean up on unmount
  return () => {
    if (socketRef.current) {
      socketRef.current.disconnect();
      socketRef.current = null;
    }
  };
}, []);

  // Initialize GridStack
useEffect(() => {
  if (gridRef.current) {
    const grid = GridStack.init({
      float: true,
      removable: true,
      removeTimeout: 100,
      acceptWidgets: true
    });

    grid.on('removed', (event, items) => {
      const removedIds = items.map(item => item.getAttribute('data-id'));
      setWidgets(prev => prev.filter(w => !removedIds.includes(w.id)));
    });

    grid.on('change', (event, items) => {
      const updatedWidgets = items.map(item => {
        const node = item.gridstackNode;
        return {
          id: item.getAttribute('data-id'),
          x: node.x,
          y: node.y,
          w: node.w,
          h: node.h
        };
      });

      setWidgets(prev =>
        prev.map(widget => {
          const updated = updatedWidgets.find(u => u.id === widget.id);
          return updated ? { ...widget, ...updated } : widget;
        })
      );
    });

    return () => {
      grid.destroy();
    };
  }
}, []); // <--- Only run once on mount

  // Update widget data when new data arrives
  const updateWidgetData = (widget, newData) => {
    let updatedOptions = { ...widget.options };
    const newDataPoint = typeof newData === 'number' ? newData : Math.random() * 100;
    const newDataArray = [...widget.data.slice(-19), newDataPoint]; // Keep last 20 points
    
    switch (widget.type) {
      case 'LINE_CHART':
        updatedOptions = {
          ...updatedOptions,
          xAxis: { ...updatedOptions.xAxis, data: newDataArray.map((_, i) => i) },
          series: [{ ...updatedOptions.series[0], data: newDataArray }]
        };
        break;
      case 'BAR_CHART':
        updatedOptions = {
          ...updatedOptions,
          series: [{ ...updatedOptions.series[0], data: newDataArray }]
        };
        break;
      case 'PIE_CHART':
        updatedOptions = {
          ...updatedOptions,
          series: [{
            ...updatedOptions.series[0],
            data: newDataArray.map((value, i) => ({
              value,
              name: `Item ${i}`
            }))
          }]
        };
        break;
      case 'GAUGE':
        updatedOptions = {
          ...updatedOptions,
          series: [{
            ...updatedOptions.series[0],
            data: [{ value: newDataPoint, name: widget.topic }]
          }]
        };
        return {
          ...widget,
          data: newDataPoint,
          options: updatedOptions
        };
      default:
        break;
    }

    return {
      ...widget,
      data: newDataArray,
      options: updatedOptions
    };
  };

  // Add a new widget to the dashboard
  const addWidget = () => {
    if (!selectedTopic) return;
    
    const newWidget = WIDGET_TYPES[selectedWidgetType].create(
      `widget-${Date.now()}`,
      selectedTopic,
      Array(5).fill(0).map(() => Math.random() * 100)
    );
    
    setWidgets([...widgets, newWidget]);
    
    // Subscribe to the topic
    if (socketRef.current) {
      socketRef.current.emit('subscribe', selectedTopic);
    }
  };

  // Save current layout
  const saveLayout = () => {
    if (!layoutName.trim()) return;
    
    const layout = {
      name: layoutName.trim(),
      widgets: [...widgets],
      timestamp: new Date().toISOString()
    };
    
    const updatedLayouts = [
      ...savedLayouts.filter(l => l.name !== layout.name),
      layout
    ];
    
    setSavedLayouts(updatedLayouts);
    localStorage.setItem('dashboardLayouts', JSON.stringify(updatedLayouts));
    setLayoutName('');
  };

  // Load a saved layout
  const loadLayout = (layoutName) => {
    const layout = savedLayouts.find(l => l.name === layoutName);
    if (layout) {
      setWidgets(layout.widgets);
    }
  };

  // Delete a saved layout
  const deleteLayout = (layoutName) => {
    const updatedLayouts = savedLayouts.filter(l => l.name !== layoutName);
    setSavedLayouts(updatedLayouts);
    localStorage.setItem('dashboardLayouts', JSON.stringify(updatedLayouts));
  };

  // Render a widget based on its type
  const renderWidget = (widget) => {
    switch (widget.type) {
      case 'LINE_CHART':
      case 'BAR_CHART':
      case 'PIE_CHART':
      case 'GAUGE':
        return (
          <ReactECharts 
            option={widget.options} 
            style={{ height: '100%', width: '100%' }} 
            notMerge={true}
          />
        );
      default:
        return <div>Unknown widget type</div>;
    }
  };

  return (
    <div className="dashboard-container">
      <div className="dashboard-controls">
        <h2>Dashboard Controls</h2>
        
        <div className="control-group">
          <label>Topic:</label>
          <select 
            value={selectedTopic} 
            onChange={(e) => setSelectedTopic(e.target.value)}
          >
            <option value="">Select a topic</option>
            {availableTopics.map(topic => (
              <option key={topic} value={topic}>{topic}</option>
            ))}
          </select>
        </div>
        
        <div className="control-group">
          <label>Widget Type:</label>
          <select
            value={selectedWidgetType}
            onChange={(e) => setSelectedWidgetType(e.target.value)}
          >
            {Object.entries(WIDGET_TYPES).map(([key, { name }]) => (
              <option key={key} value={key}>{name}</option>
            ))}
          </select>
        </div>
        
        <button onClick={addWidget}>Add Widget</button>
        
        <div className="control-group">
          <label>Save Layout:</label>
          <input
            type="text"
            value={layoutName}
            onChange={(e) => setLayoutName(e.target.value)}
            placeholder="Layout name"
          />
          <button onClick={saveLayout}>Save</button>
        </div>
      </div>
      
      <div className="saved-layouts">
        <h3>Saved Layouts</h3>
        {savedLayouts.length === 0 ? (
          <p>No saved layouts</p>
        ) : (
          <ul>
            {savedLayouts.map(layout => (
              <li key={layout.name}>
                <span>{layout.name}</span>
                <div>
                  <button onClick={() => loadLayout(layout.name)}>Load</button>
                  <button onClick={() => deleteLayout(layout.name)}>Delete</button>
                </div>
              </li>
            ))}
          </ul>
        )}
      </div>
      
      <div className="grid-stack" ref={gridRef}>
        {widgets.map(widget => (
          <div 
            key={widget.id}
            className="grid-stack-item"
            data-id={widget.id}
            gs-x={widget.x}
            gs-y={widget.y}
            gs-w={widget.w}
            gs-h={widget.h}
          >
            <div className="grid-stack-item-content">
              <div className="widget-header">
                <h3>{widget.title}</h3>
              </div>
              <div className="widget-body">
                {renderWidget(widget)}
              </div>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
};

export default Dashboard;