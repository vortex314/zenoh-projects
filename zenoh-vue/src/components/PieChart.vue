<template>
        <v-chart class="chart" :option="option" autoresize />
</template>
<script setup lang="ts">

import { ref, onMounted, watch,provide } from "vue";
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
import { messageBus } from "@/PubSub";


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
const CONFIG_DEFAULTS = {
            topic: "source topic",
            title : "just a title",
            prefix:"",
            suffix:"",
}
const emit = defineEmits(['defaultConfig'])

function messageHandler(msg) {
    console.log("msg received")
    option.value.series[0].data[0].value = Math.round(msg.value);
}

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig',CONFIG_DEFAULTS)
    messageBus.listen(props.config.topic,messageHandler);
    watch(props.config, (next,prev) =>{
      console.log(next,prev)
      messageBus.unsubscribe(prev.topic)
      messageBus.listen(next.topic,messageHandler)
    })
});

const option=ref({
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