<template>
    <v-text-field label="Label" >{{ value }}</v-text-field>
</template>

<script setup>
import { ref, onMounted, reactive, h } from 'vue'
import { messageBus } from '@/PubSub'

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

const value = ref("initial value");

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig',CONFIG_DEFAULTS)
    messageBus.listen(props.config.topic, messageHandler);

})

function messageHandler(msg) {
    console.log("msg received")
    value.value = Math.round(msg.value);
}


</script>