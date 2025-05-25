<template>
  <div>
    <button @click="addNewWidget">Add New Widget</button>
    <div class="grid-stack"> </div>
  </div>
</template>

<script setup>
import { ref, onMounted, h, onBeforeUnmount, render, inject } from "vue";
import { GridStack } from "gridstack";
import GridItem from "@/components/GridItem.vue"; // Import your Vue component

const global = inject('global'); // Inject global state if needed
let count = ref(0);
let grid = null; // DO NOT use ref(null) as proxies GS will break all logic when comparing structures... see https://github.com/gridstack/gridstack.js/issues/2115
const items = [
  { x: 0, y: 0, h: 10, w: 6, config: { title: "target RPM", src: "src/mtr1/motor/target_rpm" }, kind: "Gauge" },
  { x: 3, y: 2, h: 4, w: 3, config: { title: "measured RPM", src: "src/mtr1/motor/measure_rpm" }, kind: "LineChart" },
  { x: 4, y: 2, h: 4, w: 2, config: { title: "target RPM", src: "src/mtr1/motor/target_rpm" }, kind: "PieChart" },
  { x: 3, y: 1, h: 4, w: 4, config: { title: "target RPM", src: "src/mtr1/motor/target_rpm" }, kind: "Slider" },
  { x: 0, y: 6, w: 4, h: 2, config: { title: "target RPM", src: "src/mtr1/motor/target_rpm" }, kind: "Button" },
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
    cellHeight: "40px",
    minRow: 1,
    handle: '.card-header',
    margin: 1,
  });

  global.grid = grid; // Store grid in global state if needed
  global.items = items; // Store items in global state if needed
  global.count = count; // Store count in global state if needed


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
    console.log("Grid : renderCB", widget);

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
  }

  addNewWidget(); // Add initial widget

  //  grid.load(items, true); // load items from array
});

function addNewWidget() {
  const node = items[count.value] || {
    x: Math.round(12 * Math.random()),
    y: Math.round(5 * Math.random()),
    w: Math.round(1 + 3 * Math.random()),
    h: Math.round(1 + 3 * Math.random()),
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
  console.log(node);
  console.log()
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
