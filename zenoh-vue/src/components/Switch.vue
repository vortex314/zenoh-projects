<template>
    <v-switch v-model="switch_state" color="primary" :label="props.config.label" value="primary" hide-details
                :true-value="props.config.true_value" :false-value="props.config.false_value"
        @update:model-value="state_changed">
    </v-switch>
</template>

<script setup>
import { ref, onMounted, provide, watch, toRef, isRef, watchEffect } from "vue";
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
let subscriber = null;
function messageHandler(topic, value) {
    if (props.config.field !== "") value = value[props.config.field];
    if (props.config.eval !== "") value = eval(props.config.eval.replace('value', value))
    switch_state.value = value;
}
function state_changed() {
    bus.txd.publish(props.config.dst, switch_state.value)
}

watchEffect(() => {
    if (subscriber) subscriber.off();
    if (props.config.src) subscriber = bus.rxd.subscribe(props.config.src, messageHandler);
});

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS)
})
</script>
