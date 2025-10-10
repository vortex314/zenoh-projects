<template>
    <v-btn color="primary" size="x-small" @click="pressed" ref="id">{{ props.config.label }}</v-btn>
</template>

<script setup>
import { ref, onMounted, reactive,h  } from 'vue'
import { messageBus } from '@/PubSub'

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
    messageBus.publish(props.config.topic, props.pressed)
}

const CONFIG_DEFAULTS = {
    pressed_msg : "on",
    released_msg : "off",
    label : "Button Label",
    title :"Button Title",
    topic : "Button Topic"
}

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig',CONFIG_DEFAULTS)

})


</script>