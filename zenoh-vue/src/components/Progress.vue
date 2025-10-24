<template>
    <div>
        <span>{{ props.config.prefix }}{{ real_value }}{{ props.config.suffix }}</span>
        <v-progress-linear color="primary" :model-value="progress_value" :height="25"
            :max="100"></v-progress-linear>
        </div>
    </template>

<script setup>
import bus from "@/LocalBus";
import { ref, onMounted, provide, watch, toRef, isRef, watchEffect } from "vue";

var progress_value = ref(0)
var real_value = ref(0)
var subscriber = null;

const CONFIG_DEFAULTS = {
    src: "src/random/100",
    field: "",
    eval: "",
    title: "Mains Voltage",
    label: 'Volt',
    min: 0,
    max: 100,
    round: 1,
    step: 1,
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

watchEffect(() => {
    if (subscriber) subscriber.off();
    if (props.config.src) subscriber = bus.rxd.subscribe(props.config.src, messageHandler);
});

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS);
});

function messageHandler(topic, value) {
    if (props.config.field !== "") value = value[props.config.field];
    if (props.config.eval !== "") value = eval(props.config.eval.replace('value', value));
    real_value.value = Math.round(value);
    progress_value.value = Math.round(((value - props.config.min) / (props.config.max - props.config.min)) * 100);
}

</script>
<style scoped>
.chart {
    width: 100%;
    height: 100%;
}

.v-progress-linear {
    width: 90%;
    height: 100%;
    margin-left: 5%;
}
</style>