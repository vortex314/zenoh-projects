<template>
    <div> 
      <p>
        {{ props.config.title }}
      </p>
      <p>{{ props.config.prefix }} {{ value }} {{ props.config.suffix }}
      </p>
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
            dst: "dst/esp1/drive/HoverBoardCmd/speed",
            src:"src/esp1/drive/HoverBoardCmd/speed",
            field: "",
            eval: "",
            title : "just a title",
            prefix:"voltage : ",
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
   bus.txd.publish(props.config.dst,slider_value)
   bus.rxd.subscribe(props.config.src, (topic, newValue) => {
       value.value = newValue
   });
}


</script>