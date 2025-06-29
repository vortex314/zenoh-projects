<attachment id="file:/vue-dashboard-app/vue-dashboard-app/src/components/GaugeWidget.vue">
<template>
  <div class="gauge-widget">
    <h3>{{ title }}</h3>
    <div ref="gaugeContainer" class="gauge-container"></div>
  </div>
</template>

<script>
import { onMounted, ref } from 'vue';
import * as echarts from 'echarts';
import { subscribeToTopic } from '../services/websocket';

export default {
  name: 'GaugeWidget',
  props: {
    title: {
      type: String,
      default: 'Gauge Widget'
    },
    topic: {
      type: String,
      required: true
    }
  },
  setup(props) {
    const gaugeContainer = ref(null);
    const chartInstance = ref(null);

    const initGauge = () => {
      chartInstance.value = echarts.init(gaugeContainer.value);
      const option = {
        series: [
          {
            type: 'gauge',
            detail: { formatter: '{value}%' },
            data: [{ value: 50, name: 'Completion' }]
          }
        ]
      };
      chartInstance.value.setOption(option);
    };

    const updateGauge = (data) => {
      if (chartInstance.value) {
        chartInstance.value.setOption({
          series: [
            {
              data: [{ value: data.value }]
            }
          ]
        });
      }
    };

    onMounted(() => {
      initGauge();
      subscribeToTopic(props.topic, updateGauge);
    });

    return {
      gaugeContainer
    };
  }
};
</script>

<style scoped>
.gauge-widget {
  padding: 10px;
  border: 1px solid #ccc;
  border-radius: 5px;
  background-color: #fff;
}

.gauge-container {
  width: 100%;
  height: 200px;
}
</style>
</attachment>