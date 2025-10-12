<template>
    <span>{{ props.config.prefix }}{{ model_value }}{{ props.config.suffix }}</span>
<v-progress-linear color="primary" :model-value="model_value" :height="25"></v-progress-linear>
</template>

<script setup>
import local_bus from "@/LocalBus";
import { onMounted,ref  } from "vue";

var model_value = ref(0)

const CONFIG_DEFAULTS = {
    topic: "src/mtr1/motor.rpm_measured",
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


onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS);
    local_bus.subscribe(props.config.topic, messageHandler);
});



function messageHandler(topic, value) {
    model_value.value = Math.round(value);
}

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