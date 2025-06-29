import { reactive } from 'vue';

const websocketService = {
  socket: null,
  topics: reactive({}),
  
  connect(url) {
    this.socket = new WebSocket(url);
    
    this.socket.onopen = () => {
      console.log('WebSocket connection established');
    };
    
    this.socket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      if (this.topics[data.topic]) {
        this.topics[data.topic].forEach(callback => callback(data.payload));
      }
    };
    
    this.socket.onclose = () => {
      console.log('WebSocket connection closed');
    };
    
    this.socket.onerror = (error) => {
      console.error('WebSocket error:', error);
    };
  },
  
  subscribe(topic, callback) {
    if (!this.topics[topic]) {
      this.topics[topic] = [];
    }
    this.topics[topic].push(callback);
  },
  
  unsubscribe(topic, callback) {
    if (this.topics[topic]) {
      this.topics[topic] = this.topics[topic].filter(cb => cb !== callback);
    }
  },
  
  send(topic, message) {
    if (this.socket && this.socket.readyState === WebSocket.OPEN) {
      this.socket.send(JSON.stringify({ topic, payload: message }));
    }
  },
  
  close() {
    if (this.socket) {
      this.socket.close();
    }
  }
};

export default websocketService;