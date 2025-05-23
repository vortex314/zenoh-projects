<template>
  <div>
    <button @click="addNewWidget">Add New Widget</button>
    <p>{{ info }}</p>
    <div class="grid-stack">
      <div v-for="item in items" class="grid-stack-item">
        <div class="grid-stack-item-content">
          <div class="card-header">- Drag here - X</div>
          <div class="card">the rest of the panel content doesn't drag gfasgfagfagFHJA
            <Button :h="100"></Button>
          </div>{{ item.x }}
        </div>
      </div>
    </div>
  </div>
</template>



<script setup>
import { ref, onMounted, h, onBeforeUnmount,  render  } from "vue";
import { GridStack } from "gridstack";
import { GridItemComponent} from "./components/GridItemComponent.vue";


let count = ref(0);
let info = ref("qqqqqqqqqqqq");
let grid = null; // DO NOT use ref(null) as proxies GS will break all logic when comparing structures... see https://github.com/gridstack/gridstack.js/issues/2115
const items = [
  { x: 1, y: 1, h: 2, content: "Hello item1" },
  { x: 2, y: 4, w: 3 },
  { x: 4, y: 2 },
  { x: 3, y: 1, h: 2 },
  { x: 0, y: 6, w: 2, h: 2 },
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

  grid.on("dragstop", function (event, element) {
    const node = element.gridstackNode;
    info.value = `you just dragged node #${node.id} to ${node.x},${node.y} – good job!`;
  });

    // Listen for remove events to clean up Vue renders
  grid.on('removed', function (event, items) {
    items.forEach((item) => {
      if (shadowDom[item.id]) {
        render(null, shadowDom[item.id]);
        delete shadowDom[item.id];
      }
    });
  });

  GridStack.renderCB = function (el, widget) {
    // el: HTMLElement div.grid-stack-item-content
    // widget: GridStackWidget

    const gridItemEl = el.closest('.grid-stack-item'); // div.grid-stack-item (parent of el)

    // Create Vue component for the widget content
    const itemId = widget.id
    const widgetNode = h(GridItemComponent, {
      itemId: itemId,
      onRemove: () => { // Catch the remove event from the Vue component
        grid.removeWidget(gridItemEl); // div.grid-stack-item
        info.value = `Widget ${itemId} removed`;
      }
    })
    shadowDom[itemId] = el
    render(widgetNode, el) // Render Vue component into the GridStack-created element
  }

  grid.load(items, true); // load items from array
});

function addNewWidget() {
  const node = items[count.value] || {
    x: Math.round(12 * Math.random()),
    y: Math.round(5 * Math.random()),
    w: Math.round(1 + 3 * Math.random()),
    h: Math.round(1 + 3 * Math.random()),
  };
  node.id = node.content = String(count.value++);
  node.content = "<Button/>"
  console.log(node);
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
}

.grid-stack-item {
  border: 0px solid #000;
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
