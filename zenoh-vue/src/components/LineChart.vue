<template>
  <v-chart class="chart" :option="option" autoresize />
</template>

<script setup>
import bus from "@/LocalBus";
import { syncMappedFields  } from "@/util";
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
  legend: {}, // enables the legend

    tooltip: { trigger: 'axis' },

  xAxis: {
    type: 'category',
    data: []
  },
  yAxis: {
    type: 'value',
    min: props.config.min || 0,
    max: props.config.max || 100,
  },
  series: [
  ]
});

const CONFIG_DEFAULTS = {
  topic: "src/random/100",
  field: "",
  eval :"",
  title: "Linechart random",
  min: 0,
  max: 100,
  samples:100,
}
const emit = defineEmits(['defaultConfig', 'log'])
let subscriber = null;
let topics = [];


watchEffect(() => {
    if (subscriber) subscriber.off();
    if (props.config.topic) subscriber = bus.rxd.subscribe(props.config.topic, messageHandler);
});

function messageHandler(topic, value) {
  if (props.config.field !== "") value = value[props.config.field];
  if (props.config.eval !== "") value = eval(props.config.eval.replace('value', value));
  // find topic in topics array
  var idx = topics.indexOf(topic);
  if (idx === -1) {
    topics.push(topic);
    idx = topics.length - 1;
    option.value.series.push({
      data: [],
      type: 'line',
      name: topic+"/"+props.config.field || "value",
    });
  } 
  
  option.value.xAxis.data.push(new Date().toLocaleTimeString());

  option.value.series[idx].data.push(value);

  if (option.value.xAxis.data.length > props.config.samples) {
    option.value.xAxis.data.shift();
    option.value.series[idx].data.shift();
  }
}

onMounted(() => {
  CONFIG_DEFAULTS.id = props.id;
  emit('defaultConfig', CONFIG_DEFAULTS);
  syncMappedFields(props, option, [
      { from: 'config.max', to: 'yAxis.max' },
      { from: 'config.min', to: 'yAxis.min' },
  ])
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