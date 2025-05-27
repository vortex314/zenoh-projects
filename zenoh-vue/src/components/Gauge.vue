<template>
    <v-chart class="chart" :option="option" autoresize />
</template>

<script setup>

import { ref, onMounted, h, onBeforeUnmount, render, useTemplateRef, nextTick, provide } from "vue";
import { useElementSize } from '@vueuse/core'

import { GaugeChart } from "echarts/charts";
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
import { PieChart } from "echarts/charts";

import VChart, { THEME_KEY } from "vue-echarts";
use([
    GaugeChart,
    SVGRenderer,
    CanvasRenderer,
    TitleComponent,
    TooltipComponent,
    LegendComponent,
]);
provide(THEME_KEY, "light");

// show gauge
//import { Gauge } from "vue-google-charts";
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
        default: () => ({

            redFrom: 90,
            redTo: 100,
            yellowFrom: 75,
            yellowTo: 90,
            minorTicks: 5
        })
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

let option = ref({
    tooltip: {
        formatter: '{a} <br/>{b} : {c}%'
    },
    series: [
        {
            name: 'Pressure',
            type: 'gauge',
            progress: {
                show: true
            },
            detail: {
                valueAnimation: true,
                formatter: '{value}'
            },
            data: [
                {
                    value: 50,
                    name: 'SCORE'
                }
            ]
        }
    ]
});
const el = ref(null);

onMounted(async () => {
    await nextTick();
    console.log("Gauge", el.value);

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