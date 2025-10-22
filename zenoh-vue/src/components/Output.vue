<template>
    <div ref="container" style="width: 100%; height: 100%;">
        <v-text-field :style="{ fontSize: fontSize + 'px' }" class="text-h5">{{ props.config.prefix }} {{ value }}{{
            props.config.suffix }}</v-text-field>
    </div>
</template>

<script setup>
import { ref, onMounted, watch } from 'vue'
import bus from '@/LocalBus'

const props = defineProps({
    id: {
        type: [String, Number],
        required: true,
        default: "1"
    },
    config: {
        type: Object
    }
})
const emit = defineEmits(['defaultConfig'])
const CONFIG_DEFAULTS = {
    topic: "src/esp1/sys/SysInfo/uptime",
    field: "",
    eval: "parseFloat(value).toFixed(2)",
    title: "Output Title",
    label: "Output Label",
    suffix: "s",
    prefix: "",
}

var value = ref("---");
var fontSize = ref(12);
var container = ref(null);

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS)
    bus.rxd.subscribe(props.config.topic, messageHandler);
    adjustFontSize();
})

function messageHandler(topic, newValue) {
    if (props.config.field !== "") newValue = newValue[props.config.field];

    if (props.config.eval) {
        value.value = eval(props.config.eval.replace('value', newValue));
    } else {
        value.value = newValue;
    }
}

function adjustFontSize() { 
    if (!container.value) return; 
    const { clientWidth, clientHeight } = container.value; 
    fontSize.value = getOptimalFontSize(value.value, clientWidth, clientHeight); 
}

watch(value, () => {  adjustFontSize();});

function getOptimalFontSize(text, width, height, fontFamily = 'Arial') {
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    let size = 10;
    while (size < 200) {
        ctx.font = `${size}px ${fontFamily}`;
        const metrics = ctx.measureText(value);
        const textWidth = metrics.width;
        const textHeight = size;
        if (textWidth > width || textHeight > height) break; 
        size++;
    }
    return size - 1;
}
</script>

<style scoped></style>