<template>
  <div ref="myElement" class="grid-item">
    <div class="grid-stack-item-content" :key="item.id" :ref="item.id" :x="item.x" :y="item.y" :w="item.w" :h="item.h">
      <div class="card-header">
        <span>{{ config.title }}</span>
        <button class="remove-btn" @click="handleRemove" style="float: right;">×</button>
      </div>
      <div class="card">
        <component :is="kind" :id="props.id" :config="props.config" :dim="props.dim" />
      </div>
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

let grid = inject('global').grid; // Inject the grid instance if needed

const grid_items = {
  Gauge: markRaw(Gauge),
  Button: markRaw(Button),
  LineChart: markRaw(LineChart),
  PieChart: markRaw(PieChart),
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

// const global = inject('global');

let itemId = ref(props.id);
let item = shallowRef(props.config);

let kind = ref(grid_items[props.kind] || Gauge);

const emit = defineEmits(['remove', 'publish']);

const myElement = ref(null);

onMounted(async () => {
  await nextTick();
  console("cellHeight ", GridStack.cellHeight());
  console("cellWidth ", GridStack.cellWidth());
  // if (myElement.value) {
  // Get standard HTML attributes
  const id = myElement.value.id
  const className = myElement.value.className
  const title = myElement.value.title

  // Get data-* attributes
  const customData = myElement.value.dataset.customAttr

  // Get all attributes as a NamedNodeMap
  const allAttributes = myElement.value.attributes

  console.log({
    id,
    className,
    title,
    customData,
    allAttributes: Array.from(allAttributes).map(attr => ({
      name: attr.name,
      value: attr.value
    }))
  })
  // }

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
</style>
