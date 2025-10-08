<template>
  <div>
    <span>
      <v-btn prepend-icon="mdi-cloud-upload" color="primary" size="x-small" @click="save()" >Save</v-btn>
      <v-btn prepend-icon="mdi-cloud-download" color="primary" size="x-small" @click="load()">Load</v-btn>
      <v-btn prepend-icon="mdi-chart-line" color="primary" size="x-small" @click="addWidget('LineChart')">LineChart</v-btn>
      <v-btn prepend-icon="mdi-chart-pie" color="primary" size="x-small" @click="addWidget('PieChart')">PieChart</v-btn>
      <v-btn prepend-icon="mdi-gauge" color="primary" size="x-small" @click="addWidget('Gauge')">Gauge</v-btn>
      <v-btn prepend-icon="mdi-button" color="primary" size="x-small" @click="addWidget('Button')">v-btn</v-btn>
      <v-btn prepend-icon="mdi-table" color="primary" size="x-small" @click="addWidget('Table')">Table</v-btn>
      <v-btn prepend-icon="mdi-slider" color="primary" size="x-small" @click="addWidget('Slider')">Slider</v-btn>
      <v-btn prepend-icon="mdi-chart-bar" color="primary" size="x-small" @click="addWidget('BarChart')">BarChart</v-btn>
      <v-btn prepend-icon="mdi-text" color="primary" size="x-small" @click="addWidget('Output')">Output</v-btn>
      <v-btn prepend-icon="mdi-text" color="primary" size="x-small" @click="addWidget('YourComponent')">Props</v-btn>
    </span>
    <div class="grid-stack">
      <div v-for="(item,index) in items" :key="item.itemId" class="grid-stack-item" :gs-x="item.x" :gs-y="item.y" :gs-w="item.w"
        :gs-h="item.h" :gs-id="item.itemId" :id="item.itemId">
        <div class="grid-stack-item-content" :id="item.itemId">
          <div class="card-header">
            <span>{{ item.config.title }}</span>
            <v-btn size="x-small" class="remove-btn" @click="remove(item)" style="float: right;">X</v-btn>
          </div>
          <div class="card">
            <component :is="grid_kinds[item.kind]" :id="item.itemId" :item-id="item.itemId" :config="item.config" v-model:config="items[index].config"
              :dim="item.dim" />
          </div>
        </div>
      </div>
    </div>
  </div>

</template>

<script setup>
import { ref, markRaw, onMounted, h, onBeforeUnmount, render, inject, provide, nextTick } from "vue";
import { GridStack } from "gridstack";
import GridItem from "@/components/GridItem.vue"; // Import your Vue component
import emitter from "@/PubSub";

import Gauge from "@/components/Gauge.vue"; // Import your Vue component
import Button from "@/components/Button.vue"; // Import your Vue component
import LineChart from "@/components/LineChart.vue"; // Import your Vue component
import PieChart from "@/components/PieChart.vue"; // Import your Vue component
import Table from "@/components/Table.vue"; // Import your Vue component
import Slider from "@/components/Slider.vue"; // Import your Vue component
import Output from "./Output.vue";
import YourComponent from "./YourComponent.vue";


const grid_kinds = {
  Gauge: markRaw(Gauge),
  Button: markRaw(Button),
  LineChart: markRaw(LineChart),
  PieChart: markRaw(PieChart),
  Table: markRaw(Table),
  Slider: markRaw(Slider),
  Output: markRaw(Output),
  YourComponent:markRaw(YourComponent)
  // Slider: markRaw(() => h('div', 'Slider component not implemented yet')), // Placeholder for Slider
};

const global = ref({}); // Create a reactive global state
provide('global', global); // Provide global state if needed
let count = ref(0);
let grid = null; // DO NOT use ref(null) as proxies GS will break all logic when comparing structures... see https://github.com/gridstack/gridstack.js/issues/2115
const items = ref([
  { x: 0, y: 0, h: 20, w: 2, config: { title: "target RPM", src: "src/mtr1/motor/gauge1" }, kind: "Gauge", id: "341", itemId: "341" },
  { x: 2, y: 0, h: 20, w: 2, config: { title: "measured RPM", src: "src/mtr1/motor/line1" }, kind: "LineChart", id: "342", itemId: "342" },
  { x: 4, y: 0, h: 20, w: 2, config: { title: "target RPM", src: "src/mtr1/motor/pie1" }, kind: "PieChart", id: "343", itemId: "343" },
  { x: 6, y: 0, h: 20, w: 2, config: { title: "target RPM", src: "src/mtr1/motor/slider1" }, kind: "Slider", id: "344", itemId: "344" },
  { x: 8, y: 0, h: 20, w: 2, config: { title: "Reset", src: "src/mtr1/motor/button1" }, kind: "Button", id: "345", itemId: "345" },
  { x: 10, y: 0, h: 20, w: 2, config: { title: "target RPM", src: "src/mtr1/motor/gauge2" }, kind: "Gauge", id: "346", itemId: "346" },

]);
const shadowDom = {};

function save() {
  localStorage.setItem('grid-items', JSON.stringify(items.value));
  alert("Layout saved");
}

function load() {
  const saved = localStorage.getItem('grid-items');
  if (saved) {
    items.value = JSON.parse(saved);
    nextTick(() => {
      // Reinitialize gridstack after loading new items
      if (grid) {
        grid.destroy();
      }
      grid = GridStack.init({
        float: false,
        cellHeight: "20px",
        minRow: 1,
        handle: '.card-header',
        margin: 0,
      });
      grid.on('removed', function (event, items) {
        items.forEach((item) => {
          if (shadowDom[item.itemId]) {
            render(null, shadowDom[item.itemId]);
            delete shadowDom[item.itemId];
          }
        });
      });
      grid.on('change', onChange);
      // Make all current items widgets
      items.value.forEach((item) => {
        const el = document.getElementById(item.itemId);
        if (el) {
          grid.makeWidget(el);
        }
      });
    });
    alert("Layout loaded");
  } else {
    alert("No saved layout found");
  }
}

onBeforeUnmount(() => {
  // Clean up Vue renders
  Object.values(shadowDom).forEach((el) => {
    render(null, el);
  });
});

onMounted(() => {
  grid = GridStack.init({ // DO NOT use grid.value = GridStack.init(), see above
    float: false,
    cellHeight: "20px",
    minRow: 1,
    handle: '.card-header',
    margin: 0,
  });
//  grid.margin(5);

  // Listen for remove events to clean up Vue renders
  grid.on('removed', function (event, items) {
    items.forEach((item) => {
      if (shadowDom[item.itemId]) {
        render(null, shadowDom[item.itemId]);
        delete shadowDom[item.itemId];
      }
    });
  });

  grid.on('change', onChange);
  console.log("this emitter", emitter)

  emitter.on("publish", (data) => {
    console.log(data)
  });

   window.setInterval(() => {
           console.log(items.value[0].config);
        }, 3000);


});

function onChange(event, changeItems) {
  changeItems.forEach(item => {
    var widget = items.value.find(w => w.id == item.id);
    if (!widget) {
      alert("Widget not found: " + item.id);
      return;
    }
    widget.x = item.x;
    widget.y = item.y;
    widget.w = item.w;
    widget.h = item.h;
  });
}

function remove(widget) {
  console.log("Removing widget:", widget);
  var index = items.value.findIndex(w => w.itemId == widget.itemId);
  items.value.splice(index, 1);
  const selector = `#${widget.itemId}`;
  grid.removeWidget(widget.itemId, true, true);
}


function addWidget(kind) {
  let id = String(Math.round(2 ** 32 * Math.random()))
  let item = {
    h: 20, w: 3,  config: { title: "src/esp1/motor/MotorInfo/JSON", src: "src/mtr1/motor/target_rpm" },
    kind: kind, itemId: id, id: id
  };

  // let g = grid.addWidget(item);
    items.value.push(item);
    nextTick(() => {    
        const newEl = document.getElementById(id)
       if ( newEl ) 
        grid.makeWidget(newEl);
 })
}


</script>
<style lang="css">
/* required file for gridstack to work */
@import "gridstack/dist/gridstack.css";

.grid-stack {
  background: #FAFAD2;
}

.grid-stack-item-content {
  text-align: center;
  background-color: #18bc9c;
  width: 100% !important;
  height: 100% !important;
}

.grid-stack-item {
  border: 1px solid #000;
}

.handle-remove:hover {
  background-color: #149b80;
}

.card-header {
  margin: 0;
  cursor: move;
  min-height: 15px;
  font-size: 10px ;
  background-color: #bdbdbd;
  width: 100%;
}

.card-header:hover {
  background-color: #149b80;
}


.card {
  width: 100%;
  height: 90%;
}
</style>
