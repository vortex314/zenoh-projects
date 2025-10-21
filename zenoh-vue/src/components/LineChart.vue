<template>
  <v-chart class="chart" :option="option" autoresize />
</template>

<script setup>

import { ref, onMounted, provide, watch } from "vue";

import {
  TitleComponent,
  TooltipComponent,
  LegendComponent,
  DatasetComponent,
  GridComponent,
  ToolboxComponent,
  DataZoomComponent
} from "echarts/components";
import { use } from "echarts/core";
import {
  SVGRenderer,
  CanvasRenderer
} from "echarts/renderers";
import { LineChart } from "echarts/charts";
import VChart, { THEME_KEY } from "vue-echarts";
use([
  CanvasRenderer,
  LineChart,
  TitleComponent,
  TooltipComponent,
  LegendComponent,
  DatasetComponent,
  GridComponent,
  ToolboxComponent,
  DataZoomComponent]);
provide(THEME_KEY, "light");

import bus from "@/LocalBus";

const props = defineProps({
  id: {
    type: [String, Number],
    required: true,
    default: "1"
  },
  config: {
    type: Object
  },
});

const option = ref({
  xAxis: {
    type: 'category',
    data: []
  },
  yAxis: {
    type: 'value',
    min: props.config.min,
    max: props.config.max,
  },
  series: [
    {
      data: [],
      type: 'line'
    }
  ]
});


const CONFIG_DEFAULTS = {
  topic: "src/random/100",
  field: "",
  title: "just a title",
  min: 0,
  max: 100,
}
const emit = defineEmits(['defaultConfig', 'log'])

function messageHandler(topic, value) {
  if (props.config.field !== "") value = value[props.config.field];
  if (value > props.config.max) {
    CONFIG_DEFAULTS.max = value;
    emit('defaultConfig', CONFIG_DEFAULTS);
  }
  if (value < props.config.min) {
    CONFIG_DEFAULTS.min = value;
    emit('defaultConfig', CONFIG_DEFAULTS);
  }
  option.value.series[0].data.push(value);
  if (option.value.xAxis.data.length > 100) {
    option.value.xAxis.data.shift();
    option.value.series[0].data.shift();
  }
  option.value.xAxis.data.push(new Date().toLocaleTimeString());
}
watch(
  () => props.config,
  (newConfig, oldConfig) => {
    // Unsubscribe from the old topic
    bus.rxd.unsubscribe(oldConfig.topic, messageHandler);

    // Subscribe to the new topic
    bus.rxd.subscribe(newConfig.topic, messageHandler);

    // Update the chart options
    option.value.yAxis.min = newConfig.min;
    option.value.yAxis.max = newConfig.max;
  },
  { deep: true } // Ensure deep watching for nested properties
);


onMounted(() => {
  CONFIG_DEFAULTS.id = props.id;
  emit('defaultConfig', CONFIG_DEFAULTS);
  bus.rxd.subscribe(props.config.topic, messageHandler);
});

</script>

<style scoped>
.chart {
  width: 100%;
  height: 100%;
}

.v-chart {
  width: 100%;
  height: 100%;
}
</style>