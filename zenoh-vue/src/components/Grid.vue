<template>
  <div>
    <span>
      <button @click="addNewWidget">Add New Widget</button>
      <button @click="addWidget('LineChart')">LineChart</button>
      <button @click="addWidget('PieChart')">PieChart</button>
      <button @click="addWidget('Gauge')">Gauge</button>
    </span>
    <div class="grid-stack"> </div>
  </div>
</template>

<script setup>
import { ref, onMounted, h, onBeforeUnmount, render, inject, provide } from "vue";
import { GridStack } from "gridstack";
import GridItem from "@/components/GridItem.vue"; // Import your Vue component

const global = ref({}); // Create a reactive global state
provide('global', global); // Provide global state if needed
let count = ref(0);
let grid = null; // DO NOT use ref(null) as proxies GS will break all logic when comparing structures... see https://github.com/gridstack/gridstack.js/issues/2115
const items = [
  { x: 0, y: 0, h: 20, w: 6, config: { title: "target RPM", src: "src/mtr1/motor/target_rpm" }, kind: "Gauge" },
  { x: 6, y: 0, h: 20, w: 5, config: { title: "measured RPM", src: "src/mtr1/motor/measure_rpm" }, kind: "LineChart" },
  { x: 4, y: 2, h: 20, w: 5, config: { title: "target RPM", src: "src/mtr1/motor/target_rpm" }, kind: "PieChart" },
  { x: 3, y: 1, h: 20, w: 4, config: { title: "target RPM", src: "src/mtr1/motor/target_rpm" }, kind: "Slider" },
  { x: 0, y: 6, w: 20, h: 6, config: { title: "target RPM", src: "src/mtr1/motor/target_rpm" }, kind: "Button" },
];
const shadowDom = {};

onBeforeUnmount(() => {
  // Clean up Vue renders
  Object.values(shadowDom).forEach((el) => {
    render(null, el);
  });
});

onMounted(() => {
  grid = GridStack.init({ // DO NOT use grid.value = GridStack.init(), see above
    float: true,
    cellHeight: "20px",
    minRow: 1,
    handle: '.card-header',
    margin: 2,
  });

  global.value.grid = grid; // Store grid in global state if needed
  global.value.items = items; // Store items in global state if needed
  global.value.count = count; // Store count in global state if needed


  // Listen for remove events to clean up Vue renders
  grid.on('removed', function (event, items) {
    items.forEach((item) => {
      if (shadowDom[item.id]) {
        render(null, shadowDom[item.id]);
        delete shadowDom[item.id];
      }
    });
  });

  grid.on('change', function (event, items) {
    //    console.log("GridStack change event", items);
  });


  GridStack.renderCB = function (el, widget) {
    console.log("Grid : renderCB", widget, "[", grid.getCellHeight(), ",", grid.cellWidth(), "]");

    const gridItemEl = el.closest('.grid-stack-item'); // div.grid-stack-item (parent of el)

    // Create Vue component for the widget content
    const id = widget.id
    const widgetNode = h(GridItem, {
      id: id,
      dim: widget.dim,
      config: widget.config,
      kind: widget.kind || "Gauge",
      onRemove: () => { // Catch the remove event from the Vue component
        grid.removeWidget(gridItemEl); // div.grid-stack-item
        info.value = `Widget ${id} removed`;
      }
    })
    shadowDom[id] = el
    render(widgetNode, el) // Render Vue component into the GridStack-created element
    //   el.style.height = String(widget.dim.h * grid.getCellHeight()) + "px";
    //   el.style.width = String(widget.dim.w * grid.cellWidth()) + "px";
    console.log("Element data", el)
    console.log("style after ", el.style)
    this.emitter.on("publish", (data) => {
      console.log(data)
    });
  }

  addNewWidget(); // Add initial widget

  //  grid.load(items, true); // load items from array
});

function addNewWidget() {
  const node = items[count.value] || {
    x: 6,
    y: 10,
    w: 4,
    h: 6,
  };
  let dim = {
    x: (node.x === undefined) ? 0 : node.x,
    y: (node.y === undefined) ? 0 : node.y,
    w: (node.w === undefined) ? 1 : node.w,
    h: (node.h === undefined) ? 1 : node.h,
  }
  node.id = String(Math.round(2 ** 32 * Math.random()))
  node.content = String(count.value++);
  node.dim = dim;
  grid.addWidget(node);
}

function addWidget(kind) {
  this.emitter.emit("publish", { topic: "dst/mtr1/motor.rpm_target", value: 3140 })
  const node = items[count.value] || {
    x: 6,
    y: 6,
    w: 4,
    h: 6,
  };
  let dim = {
    x: (node.x === undefined) ? 0 : node.x,
    y: (node.y === undefined) ? 0 : node.y,
    w: (node.w === undefined) ? 1 : node.w,
    h: (node.h === undefined) ? 1 : node.h,
  }
  node.id = String(Math.round(2 ** 32 * Math.random()))
  node.content = String(count.value++);
  node.dim = dim;
  node.kind = kind || "Gauge"; // Set the kind of widget
  grid.addWidget(node);
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
  border: 0px solid #000;
}
</style>
