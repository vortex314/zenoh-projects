<template>
        <v-chart class="chart" :option="option" autoresize />
</template>

<script setup lang="ts">

import { ref, onMounted, h, onBeforeUnmount, render, useTemplateRef, nextTick, provide } from "vue";
import { useElementSize } from '@vueuse/core'

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
  locked: {
    type: Boolean,
    default: false
  },
  options: {
    type: Object,
    default: () => ({})
  },
  dim: {
    type: Object,
    default: () => ({})
  },
  kind: {
    type: String,
    default: "Gauge"
  }
});
onMounted(() => {
    console.log("Gauge"," component mounted");
});

const option=ref({
  title: {
    text: 'Referer of a Website',
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