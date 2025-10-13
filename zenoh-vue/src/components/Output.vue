<template>
    <v-text-field class="white-text" >.{{ props.config.prefix }} {{ value }}{{ props.config.suffix }}</v-text-field>
</template>

<script setup>
import { ref, onMounted, reactive, h } from 'vue'
import local_bus from '@/LocalBus'

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
    title :"Output Title",
    label : "Output Label",
    suffix : "s",
    prefix : "",
}

var value = ref("---");

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig',CONFIG_DEFAULTS)
    local_bus.subscribe(props.config.topic, messageHandler);
})

function messageHandler(topic, newValue) {
    value.value = topic +'=' +JSON.stringify(Math.round(newValue));
    console.log("props ",props.config.prefix,props.config.suffix);
}


</script>

<style scoped>
.white-text {
    color: white !important;
}
</style>