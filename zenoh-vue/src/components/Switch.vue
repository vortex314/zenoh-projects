<template>
    <v-switch v-model="switch_state" color="primary" :label="props.config.label" value="primary" hide-details
                :true-value="props.config.true_value" :false-value="props.config.false_value"
        @update:model-value="state_changed">
    </v-switch>
</template>

<script setup>
import { ref, onMounted } from 'vue'
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
const CONFIG_DEFAULTS = {
    dst: "dst/null/switch",
    src: "src/null/switch",
    field :"",
    title: "Switch Title",
    label: "Switch Label",
    true_value: true,
    false_value: false,
}
const emit = defineEmits(['defaultConfig'])
const switch_state = ref(true)

function state_changed() {
    bus.txd.publish(props.config.dst, switch_state.value)
}



onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS)
    bus.rxd.subscribe(props.config.src, (topic, value) => {
        var v = JSON.stringify(value); // the config is translated to string 
        switch_state.value = v;
    });
})
/*

        :true-value="props.config.true_value" :false-value="props.config.false_value"

        */

</script>
