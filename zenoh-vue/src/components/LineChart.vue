<template>
        <v-chart class="chart" :option="option" autoresize />
</template>

<script setup lang="ts">

import { ref, onMounted, h, onBeforeUnmount, render, useTemplateRef, nextTick, provide } from "vue";

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

import  local_bus  from "@/LocalBus";

const option = ref({
  xAxis: {
    type: 'category',
    data: []
  },
  yAxis: {
    type: 'value'
  },
  series: [
    {
      data: [],
      type: 'line'
    },
    {
      data: [],
      type: 'line'
    },
    {
      data: [],
      type: 'line'
    }
  ]
});

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
const CONFIG_DEFAULTS = {
            topic: "src/mtr1/motor.rpm_target",
            title : "just a title",
}
const emit = defineEmits(['defaultConfig'])
onMounted(() => {
  CONFIG_DEFAULTS.id = props.id
 emit('defaultConfig',CONFIG_DEFAULTS)
  local_bus.subscribe("src/mtr1/motor.rpm_target", (topic,value) => {
            option.value.series[0].data.push(Math.round(value*2));
            option.value.series[1].data.push(Math.round(value*Math.random()*2));
            option.value.series[2].data.push(Math.round(value*Math.random()*2));

            if (option.value.xAxis.data.length > 100) {
                option.value.xAxis.data.shift();
                option.value.series[0].data.shift();
                option.value.series[1].data.shift();
                option.value.series[2].data.shift();
            }

            option.value.xAxis.data.push(new Date().toLocaleTimeString());
    });
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