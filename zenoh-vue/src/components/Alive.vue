<template>
    <v-switch v-model="switch_state" color="primary" :label="props.config.label" value="primary" hide-details
                :true-value="props.config.true_value" :false-value="props.config.false_value"
        @update:model-value="state_changed">
    </v-switch>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import local_bus from '@/LocalBus'

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
const alive_state = ref([])

function state_changed() {
    local_bus.publish(props.config.dst, switch_state.value)
}

const CONFIG_DEFAULTS = {
    src: "src/random/bool",
    title: "Alive",
    label: "Switch Label",
}

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS)
    local_bus.subscribe(props.config.src, (topic, value) => {
        var v = JSON.stringify(value); // the config is translated to string 
        switch_state.value = v;
    });
})
</script>
