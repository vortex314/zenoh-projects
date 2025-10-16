<template>
  <v-chart class="chart" :option="option" autoresize />
</template>
<script setup>

import { ref, onMounted, watch, provide } from "vue";

import { PieChart } from "echarts/charts";
import {
  TitleComponent,
  TooltipComponent,
  LegendComponent,
} from "echarts/components";
import { use } from "echarts/core";
import {
  SVGRenderer,
  CanvasRenderer
} from "echarts/renderers";
import  local_bus  from "@/LocalBus";


import VChart, { THEME_KEY } from "vue-echarts";
use([
  PieChart,
  SVGRenderer,
  CanvasRenderer,
  TitleComponent,
  TooltipComponent,
  LegendComponent,
]);
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
const emit = defineEmits(['defaultConfig'])


const CONFIG_DEFAULTS = {
  topic: "source topic",
  title: "just a title",
  prefix: "",
  suffix: "",
}

function messageHandler(value) {
  option.value.series[0].data[0].value = Math.round(value);
}

onMounted(() => {
  CONFIG_DEFAULTS.id = props.id;
  emit('defaultConfig', CONFIG_DEFAULTS);
  local_bus.subscribe(props.config.topic, messageHandler);
  watch(props.config, (next, prev) => {
    console.log(next, prev)
    local_bus.unsubscribe(prev.topic, messageHandler);
    local_bus.subscribe(next.topic, messageHandler);
  })
});

const option = ref({
  title: {
    text: props.config.title,
    subtext: 'Fake Data',
    left: 'center'
  },
  tooltip: {
    trigger: 'item'
  },
  legend: {
    orient: 'vertical',
    left: 'left'
  },
  series: [
    {
      name: 'Access From',
      type: 'pie',
      radius: '50%',
      data: [
        { value: 1048, name: 'Search Engine' },
        { value: 735, name: 'Direct' },
        { value: 580, name: 'Email' },
        { value: 484, name: 'Union Ads' },
        { value: 300, name: 'Video Ads' }
      ],
      emphasis: {
        itemStyle: {
          shadowBlur: 10,
          shadowOffsetX: 0,
          shadowColor: 'rgba(0, 0, 0, 0.5)'
        }
      }
    }
  ]
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