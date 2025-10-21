<template>
    <v-chart class="chart" :option="option" autoresize />
</template>

<script setup>

import { ref, onMounted, provide, watch } from "vue";
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

import local_bus from "@/LocalBus";


const CONFIG_DEFAULTS = {
    topic: "src/device/component/message_type/property",
    field :"",
    title: "Mains Voltage",
    label: 'Volt',
    min: 0.0,
    max: 100.0,
    round: 1,
    step: 10,
    prefix: "",
    suffix: "",
    minorTicks: 1,
    majorTicks: 5,
    gaugeColor: "#00ff00",
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
    tooltip: {
        formatter: '{a} <br/>{b} : {c}%'
    },
    series: [
        {
            name: props.config.title,
            min : props.config.min,
            max : props.config.max,
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
                    name: props.config.label,
                }
            ]
        }
    ]
});

function messageHandler(topic, value) {
    if ( props.config.field !== "") value = value[props.config.field];
    option.value.series[0].data[0].value = Math.round(value);
}

watch(() => props.config,(new_prop,old_prop) => {
    console.log("Props changed ",old_prop," to ",new_prop);
});
onMounted(() => {
    CONFIG_DEFAULTS.id = props.id;
    emit('defaultConfig', CONFIG_DEFAULTS);
    local_bus.subscribe(props.config.topic, messageHandler);
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