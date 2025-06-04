<template>
    <div> {{ value }}
        <v-slider v-model="value" thumb-label ref="id">BUTTON</v-slider>
    </div>
</template>

<script setup>
import { ref, onMounted, reactive, h } from 'vue'
import { PubSub } from '@/PubSub'

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
const value = ref(50)

function pressed() {
    PubSub.publish(props.src, props.pressed)
}

onMounted(() => {

})


</script>