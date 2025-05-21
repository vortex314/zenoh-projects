<template>
      <button type="button" @click="addNewWidget()">Add Widget</button> {{ info }}
      <div class="grid-stack"></div>
</template>
<script src="gridstack/dist/gridstack-all.js"></script>

<script setup lang="ts">
import { ref, onMounted  } from 'vue'
import { GridStack } from 'gridstack';


let count = ref(0);
let info = ref("");
let grid = null; // DO NOT use ref(null) as proxies GS will break all logic when comparing structures... see https://github.com/gridstack/gridstack.js/issues/2115
const items = [
    { x: 2, y: 1, h: 2 },
    { x: 2, y: 4, w: 3 },
    { x: 4, y: 2 },
    { x: 3, y: 1, h: 2 },
    { x: 0, y: 6, w: 2, h: 2 },
];

 onMounted(()=>  {
    grid = GridStack.init({ // DO NOT use grid.value = GridStack.init(), see above
        float: true,
        cellHeight: "70px",
        minRow: 1,
    });

    grid.on("dragstop", function (event, element) {
        const node = element.gridstackNode;
        info.value = `you just dragged node #${node.id} to ${node.x},${node.y} – good job!`;
    });
});

function addNewWidget() {
    const node = items[count.value] || {
        x: Math.round(12 * Math.random()),
        y: Math.round(5 * Math.random()),
        w: Math.round(1 + 3 * Math.random()),
        h: Math.round(1 + 3 * Math.random()),
    };
    node.id = node.content = String(count.value++);
    grid.addWidget(node);
}

</script>

<style lang="css" scoped>
@import "../assets/demo.css"
</style>