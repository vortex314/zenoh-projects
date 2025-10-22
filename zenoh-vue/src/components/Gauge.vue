<template>
    <v-chart class="chart" :option="option" autoresize />
</template>

<script setup>

import { ref, onMounted, provide, watch, toRef, watchEffect } from "vue";
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


const CONFIG_DEFAULTS = {
    topic: "src/random/100",
    field: "",
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
            name: "Title",
            min: 0,
            max: 100,
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
                    name: "Label"
                }
            ]
        }
    ]
});

const series = toRef(option.value.series[0]);

function messageHandler(topic, value) {
    console.log("Gauge received:", props.config.topic,topic, value);
    if (props.config.field !== "") value = value[props.config.field];
    option.value.series[0].data[0].value = Math.round(value);
}

watchEffect(() => {
    console.log("Gauge watchEffect:", props.config.min, props.config.max);
    option.value.series[0].max = props.config.max;
    option.value.series[0].min = props.config.min;
    option.value.series[0].name = props.config.title;
    option.value.series[0].data[0].name =  props.config.label;
});

/*
watch(
  () => [props.config.min, props.config.max],
  ([min, max]) => {
    option.value.series[0].min = min
    option.value.series[0].max = max
  }
)*/
/*
watch(() => props.config,(new_prop,old_prop) => {
    console.log("Gauge config changed:", old_prop.topic, "->", new_prop.topic);
    bus.rxd.unsubscribe(old_prop.topic, messageHandler);
    bus.rxd.subscribe(new_prop.topic, messageHandler);
    option.value.series[0].name = new_prop.title || option.value.series[0].name;
    option.value.series[0].min = new_prop.min || option.value.series[0].min;
    option.value.series[0].max = new_prop.max || option.value.series[0].max;
    option.value.series[0].data[0].name = new_prop.label || option.value.series[0].data[0].name;
});*/
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