<template>
    <v-text-field >{{ props.config.prefix }} {{ value }}{{ props.config.suffix }}</v-text-field>
</template>

<script setup>
import { ref, onMounted, reactive, h } from 'vue'
import bus from '@/LocalBus'

const props = defineProps({
    id: {
        type: [String, Number],
        required: true,
        default: "1"
    },
    config:{
        type:Object
    }
})
const emit = defineEmits(['defaultConfig'])
const CONFIG_DEFAULTS = {
    topic : "src/esp1/sys/SysInfo/uptime",
    field:"",
    eval:"parseFloat(value).toFixed(2)",
    title :"Output Title",
    label : "Output Label",
    suffix : "s",
    prefix : "",
}

var value = ref("---");

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig',CONFIG_DEFAULTS)
    bus.rxd.subscribe(props.config.topic, messageHandler);
})

function messageHandler(topic, newValue) {
    if (props.config.field !== "") newValue = newValue[props.config.field];
    
    if (props.config.eval) {
        value.value = eval(props.config.eval.replace('value', newValue));
    } else {
        value.value = newValue;
    }
}
</script>

<style scoped>

</style>