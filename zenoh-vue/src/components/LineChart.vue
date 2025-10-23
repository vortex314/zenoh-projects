<template>
  <v-chart class="chart" :option="option" autoresize />
</template>

<script setup>
import bus from "@/LocalBus";
import { ref, onMounted, provide, watchEffect } from "vue";
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
    min: 0,
    max: 100,
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
  title: "Linechart random",
  min: 0,
  max: 100,
  samples:100,
}
const emit = defineEmits(['defaultConfig', 'log'])

watchEffect(() => {
    console.log("LineChart watchEffect:", props.config.min, props.config.max);
    option.value.yAxis.min = props.config.min;
    option.value.yAxis.max = props.config.max;
});

function messageHandler(topic, value) {
 // console.log("LineChart received:", topic, value);
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
  if (option.value.xAxis.data.length > props.config.samples) {
    option.value.xAxis.data.shift();
    option.value.series[0].data.shift();
  }
  option.value.xAxis.data.push(new Date().toLocaleTimeString());
}

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