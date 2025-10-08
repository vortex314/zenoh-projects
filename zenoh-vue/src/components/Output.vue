<template>
    <v-text-field label="Label" >{}</v-text-field>
</template>

<script setup>
import { ref, onMounted, reactive, h } from 'vue'
import { messageBus } from '@/PubSub'
import { useEventBus } from '@vueuse/core'

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
    label: {
        type: String,
        default: "Reset"
    },
    config:{
        type:Object,
        default: () => ({
            prefix:"",
            suffix:"",
        })
    }
})
const id = ref(null)


onMounted(() => {
    

})


</script>