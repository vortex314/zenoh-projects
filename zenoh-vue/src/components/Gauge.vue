<template>
    <v-chart class="chart" :option="option" autoresize />
</template>

<script setup>
import { ref, onMounted, provide, watch, toRef, isRef, watchEffect } from "vue";
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
import { GaugeChart } from "echarts/charts";
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

import bus from "@/LocalBus";
import { syncMappedFields } from "@/util";

const CONFIG_DEFAULTS = {
    src: "src/random/100",
    field: "",
    eval: "",
    title: "Mains Voltage",
    label: 'Volt',
    min: 0,
    max: 100,
    round: 1,
    step: 10,
    prefix: "",
    suffix: "",
    minorTicks: 1,
    majorTicks: 5,
    gaugeColor: "#00ff00",
    icon: "mdi-gauge",
}
const emit = defineEmits(['defaultConfig'])

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
let option = ref({
    title: {
        text: props.config.title || "Gauge",   // 👈 main title
        left: 'center',
    },
    tooltip: {
        formatter: '{a} <br/>{b} : {c}'
    },
    series: [
        {
            
            min: props.config.min || 0,
            max: props.config.max || 100,
            type: 'gauge',
            progress: {
                show: true
            },
            detail: {
                valueAnimation: false,
                formatter: '{value}'
            },
            data: [
                {
                    value: 50,
                    name: props.config.title || "Gauge"
                }
            ]
        }
    ]
});
let subscriber = null;

function messageHandler(topic, value) {
    if (props.config.field !== "") value = value[props.config.field];
    if (props.config.eval !== "") value = eval(props.config.eval.replace('value', value))
    option.value.series[0].data[0].value = Math.round(value);
}

watchEffect(() => {
    if (subscriber) subscriber.off();
    if (props.config.src) subscriber = bus.rxd.subscribe(props.config.src, messageHandler);
});

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id;
    emit('defaultConfig', CONFIG_DEFAULTS);
    syncMappedFields(props, option, [
        { from: 'config.max', to: 'series[0].max' },
        { from: 'config.min', to: 'series[0].min' },
        { from: 'config.title', to: 'series[0].name' },
        { from: 'config.label', to: 'series[0].data[0].name' },
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