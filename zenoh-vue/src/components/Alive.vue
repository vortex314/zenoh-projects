<template>
    <span style="display: block;" :class="alive_state" width="100%">{{ props.config.label }}</span>
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
const alive_state = ref("dead")
const last_update = ref(Date.now())
const found_topics=ref({})



const CONFIG_DEFAULTS = {
    src: "src/random/bool",
    title: "Alive",
    label: "Switch Label",
    timeout: 5000,
}

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS)
    local_bus.subscribe(props.config.src, (topic, value) => {
        if ( topic in found_topics ) {

        } else 
        {
            
        }
        alive_state.value = "alive"
        last_update.value = Date.now()
    });
    setInterval(() => {
        console.log("check alive ", Date.now() - last_update.value, props.config.timeout);
        if (Date.now() - last_update.value > props.config.timeout) {
            alive_state.value = "dead"
        }
    }, props.config.timeout)
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
