<template>
    <v-btn color="primary" size="x-small" @click="pressed" ref="id">BUTTON</v-btn>
</template>

<script setup>
import { ref, onMounted, reactive,h  } from 'vue'
import { messageBus } from '@/PubSub'

const props = defineProps({
    h: {
        type: Number,
        default: 200
    }, w: {
        type: Number,
        default: 200
    }, src: {
        type: String,
        default: "dst/esp32/sys/reset"
    },
    pressed: {
        type: String,
        default: "on"
    },
    released: {
        type: String,
        default: "off"
    },
    label: {
        type: String,
        default: "Reset"
    }
})
const id = ref(null)

function pressed() {
    messageBus.publish(props.src, props.pressed)
}

onMounted(() => {


})


</script>