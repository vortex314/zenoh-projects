<template>
        <div style="display: block;" :class="alive_state" width="100%">{{ props.config.label }}</div>
</template>

<script setup>
import { ref, onMounted, provide, watch, toRef, isRef, watchEffect } from "vue";
import bus from "@/LocalBus";
import { syncMappedFields } from "@/util";
let subscriber = null;

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
const alive_state = ref("dead")
const last_update = ref(Date.now())
const CONFIG_DEFAULTS = {
    src: "src/random/bool",
    title: "Alive",
    label: "Alive Status",
    timeout: 5000,
}

watchEffect(() => {
    if (subscriber) subscriber.off();
    if (props.config.src) subscriber = bus.rxd.subscribe(props.config.src, messageHandler);
});

function messageHandler(topic, value) {
    alive_state.value = "alive"
    last_update.value = Date.now()
}

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS)
    bus.rxd.subscribe(props.config.src, messageHandler);
    setInterval(() => {
        if (Date.now() - last_update.value > props.config.timeout) {
            alive_state.value = "dead"
        }
    }, props.config.timeout)
    syncMappedFields(props, {}, [
        { from: 'config.label', to: 'label' },
    ])
})
</script>

<style scoped>
span {
    display: block;
    text-align: center;
    font-weight: bold;
    font-size-adjust: ex-height 0.5;
    line-height: 100px;
    height: 100%;
    width: 100%;
}

.dead {
    background-color: red;
}

.alive {
    background-color: #90EE90;
}

.unknown {
    background-color: gray;
}
</style>
