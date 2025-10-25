<template>
    <div ref="container" style="width: 100%; height: 100%;">
        <v-text-field :style="{ fontSize: fontSize + 'px' }" class="text-h5">{{ props.config.prefix }} {{ text_value }}{{
            props.config.suffix }}</v-text-field>
    </div>
</template>

<script setup>
import { ref, onMounted, watch,watchEffect } from 'vue'
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
    src: "src/esp1/sys/SysInfo",
    field: "uptime",
    eval: "new Date(value).toISOString().slice(11,19) ",
    title: "Output Title",
    label: "Output Label",
    suffix: "s",
    prefix: "",
}

var text_value = ref("---");
var fontSize = ref(12);
var container = ref(null);
let subscriber = null;

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS)
})

watchEffect(() => {
    if (subscriber) subscriber.off();
    if (props.config.src) subscriber = bus.rxd.subscribe(props.config.src, messageHandler);
});

function messageHandler(topic, value) {
    console.log("Output received:", topic, value);
    if (props.config.field !== "") value = value[props.config.field];
    if (props.config.eval !== "") value = eval(props.config.eval.replace('value', value));    
    text_value.value = value; 
}

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