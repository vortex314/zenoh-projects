<template>
    <v-chart class="chart" :option="option" autoresize />
</template>

<script setup>

import { ref, onMounted, h, onBeforeUnmount, render, useTemplateRef, nextTick, provide } from "vue";

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

import { messageBus } from "@/PubSub";

const modelValue = defineModel()  // Vue 3.4+ syntax

// show gauge
//import { Gauge } from "vue-google-charts";
const props = defineProps({
    id: {
        type: [String, Number],
        required: true,
        default: "1"
    },
    config: {
        type: Object,
        default: () => ({
            topic: "src/device/component/message_type/property",
            min: 0.0,
            max: 100.0,
            round:1,
            step: 10,
            title : topic,
            prefix:"",
            suffix:"",
            minorTicks:1,
            majorTicks:5,
        })
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
let cfg = defineModel('config')  // This is reactive and two-way bound
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

onMounted(() => {
    console.log("Modelvalue Gauge ", cfg);
    cfg.value = props.config;
    console.log("props:", props.config);
    messageBus.listen("src/mtr1/motor.rpm_target", (msg) => {
        option.value.series[0].data[0].value = Math.round(msg.value);
    });
       window.setInterval(() => {
           console.log(cfg);
        }, 3000);
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