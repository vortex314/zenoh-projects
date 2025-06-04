<template>
    <div class="grid-stack-item-content" :key="item.id" :ref="item.id" :x="item.x" :y="item.y" :w="item.w" :h="item.h">
      <div class="card-header">
        <span>{{ config.title }}</span>
        <button class="remove-btn" @click="handleRemove" style="float: right;">×</button>
      </div>
      <div class="card" >
        <component :is="kind" :id="props.id" :config="props.config" :dim="props.dim" />
      </div>
    </div>
</template>

<script setup>
import {
  ref, onMounted, h, onBeforeUnmount, render, inject,
  shallowRef, markRaw, nextTick,
} from "vue";
import { GridStack } from "gridstack"; // Import GridStack if needed
import Gauge from "@/components/Gauge.vue"; // Import your Vue component
import Button from "@/components/Button.vue"; // Import your Vue component
import LineChart from "@/components/LineChart.vue"; // Import your Vue component
import PieChart from "@/components/PieChart.vue"; // Import your Vue component
import Table from "@/components/Table.vue"; // Import your Vue component

const grid_kinds = {
  Gauge: markRaw(Gauge),
  Button: markRaw(Button),
  LineChart: markRaw(LineChart),
  PieChart: markRaw(PieChart),
  Table: markRaw(Table),
  Slider: markRaw(() => h('div', 'Slider component not implemented yet')), // Placeholder for Slider
};

const props = defineProps({
  id: {
    type: [String, Number],
    required: true,
    default: "1"
  },
  locked: {
    type: Boolean,
    default: false
  },
  config: {
    type: Object,
    default: () => ({})
  },
  dim: {
    type: Object,
    default: () => ({})
  },
  kind: {
    type: String,
    default: "Gauge"
  }
});

let itemId = ref(props.id);
let item = shallowRef(props.config);

let kind = ref(grid_kinds[props.kind] || Gauge);
const emit = defineEmits(['remove', 'publish']);

onMounted( () => {
  console.log("creating GridItem ", kind.value)
  item.value = { ...item.value, ...props.options };
});

onBeforeUnmount(() => {
  console.log(`In vue onBeforeUnmount for item `, JSON.stringify(props));
});

function handleRemove() {
  if (!props.locked) {
    emit('remove');
  }
}

</script>

<style scoped>
/* Add your styles here */
.handle-remove:hover {
  background-color: #149b80;
}

.card-header {
  margin: 0;
  cursor: move;
  min-height: 18px;
  background-color: #bdbdbd;
  width: 100%;
}

.card-header:hover {
  background-color: #149b80;
}

.grid-stack-item-content {
  display :flex; 
  flex-direction: column; 
  justify-content: top; 
  align-items: center;
  background-color: #0e3516;
  border: 1px solid #ccc;
  border-radius: 4px;
}

.card {
  width: 100%;
  height: 100%;
}
</style>
