export const topics = {
  CHART_DATA: 'chart/data',
  GAUGE_DATA: 'gauge/data',
  GENERAL_UPDATES: 'general/updates',
};

export const subscribeToTopic = (websocket, topic, callback) => {
  websocket.send(JSON.stringify({ action: 'subscribe', topic }));
  websocket.onmessage = (event) => {
    const data = JSON.parse(event.data);
    if (data.topic === topic) {
      callback(data.payload);
    }
  };
};

export const unsubscribeFromTopic = (websocket, topic) => {
  websocket.send(JSON.stringify({ action: 'unsubscribe', topic }));
};