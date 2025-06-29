<attachment id="file:ChartWidget.vue">
<template>
  <div class="chart-widget">
    <div ref="chart" class="chart"></div>
  </div>
</template>

<script>
import { onMounted, onBeforeUnmount, ref } from 'vue';
import * as echarts from 'echarts';
import { subscribeToTopic } from '../services/websocket';

export default {
  name: 'ChartWidget',
  props: {
    topic: {
      type: String,
      required: true
    }
  },
  setup(props) {
    const chart = ref(null);
    const chartInstance = ref(null);

    const initChart = () => {
      chartInstance.value = echarts.init(chart.value);
      chartInstance.value.setOption({
        title: {
          text: 'Dynamic Chart'
        },
        tooltip: {},
        xAxis: {
          data: []
        },
        yAxis: {},
        series: [{
          name: 'Data',
          type: 'line',
          data: []
        }]
      });
    };

    const updateChart = (data) => {
      const option = chartInstance.value.getOption();
      option.series[0].data.push(data.value);
      option.xAxis[0].data.push(data.timestamp);
      chartInstance.value.setOption(option);
    };

    onMounted(() => {
      initChart();
      subscribeToTopic(props.topic, updateChart);
    });

    onBeforeUnmount(() => {
      if (chartInstance.value) {
        chartInstance.value.dispose();
      }
    });

    return {
      chart
    };
  }
};
</script>

<style scoped>
.chart-widget {
  width: 100%;
  height: 100%;
}
.chart {
  width: 100%;
  height: 100%;
}
</style>
</attachment>