<template>
    <button @click="pressed" ref="id" :width="w" :height="h">{{ label }}</button>
</template>

<script setup>
import { ref, onMounted, reactive } from 'vue'

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
    console.log("pressed")
    publish(props.src, props.pressed)
}

onMounted(() => {
    console.log(props)
})


</script>