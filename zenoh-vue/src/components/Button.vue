<template>
    <v-btn block class="w-100 h-100" color="primary" size="x-small" @click="pressed" ref="id">{{ props.config.label }}</v-btn>
</template>

<script setup>
import { ref, onMounted  } from 'vue'
import bus from '@/LocalBus'

const props = defineProps({
    config: {
        type: Object,
    },
    id: {
        type: [String, Number],
        required: true,
        default: "1"
    },
})
const emit = defineEmits(['defaultConfig'])
const id = ref(null)

function pressed() {
    bus.txd.publish(props.config.topic, props.config.pressed_msg)
}

const CONFIG_DEFAULTS = {
    topic : "dst/esp1/sys/SysCmd/reset",
    title :"Button Title",
    label : "Button Label",
    pressed_msg : "true",
    released_msg : "false",

}

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig',CONFIG_DEFAULTS)

})


</script>

<style scoped>

</style>