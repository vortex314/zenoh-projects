<template>
    <div> {{ value }}
        <v-slider thumb-label 
            v-model="value" 
            :step="props.config.step" 
            :max="props.config.max" 
            :min="props.config.min"  
            ref="id"
            color="primary"
            @change="onChange"
            @end="onChange"></v-slider>
    </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import bus from '@/LocalBus'

const props = defineProps({
  id: {
    type: [String, Number],
    required: true,
    default: "1"
  },
  config: {
        type: Object
  },
})
const value = ref(51)
const CONFIG_DEFAULTS = {
            topic: "dst/esp1/drive/HoverBoardCmd/speed",
            title : "just a title",
            prefix:"",
            suffix:"V",
            min:-100,
            max:100,
            step:10,
}
const emit = defineEmits(['defaultConfig'])

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig',CONFIG_DEFAULTS)
})

function onChange(slider_value) {
   bus.txd.publish(props.config.topic,slider_value)
}


</script>