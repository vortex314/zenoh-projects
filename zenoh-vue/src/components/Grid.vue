<template>
  <div>
    <v-container>
      <v-system-bar window="true" color="primary" style="align-content: left">
          {{ log_message }}
        <v-icon icon="mdi-cloud-upload" class="ms-2" @click="save()" hover="Save Dashboard"></v-icon>
        <v-icon icon="mdi-cloud-download" class="ms-2" @click="load()"></v-icon>
        <v-icon icon="mdi-chart-line" class="ms-2" @click="addWidget('LineChart')"></v-icon>
        <v-icon icon="mdi-chart-pie" class="ms-2" @click="addWidget('PieChart')"></v-icon>
        <v-icon icon="mdi-gauge" class="ms-2" @click="addWidget('Gauge')"></v-icon>
        <v-icon icon="mdi-button-cursor" class="ms-2" @click="addWidget('Button')"></v-icon>
        <v-icon icon="mdi-table" class="ms-2" @click="addWidget('Table')"></v-icon>
        <v-icon icon="mdi-tune-variant" class="ms-2" @click="addWidget('Slider')"></v-icon>
        <v-icon icon="mdi-chart-bar" class="ms-2" @click="addWidget('BarChart')"></v-icon>
        <v-icon icon="mdi-text" class="ms-2" @click="addWidget('Output')"></v-icon>
        <v-icon icon="mdi-arrow-expand-right" class="ms-2" @click="addWidget('Progress')"></v-icon>
        <v-icon icon="mdi-toggle-switch" class="ms-2" @click="addWidget('Switch')"></v-icon>
        <v-icon icon="mdi-text" class="ms-2" @click="addWidget('YourComponent')"></v-icon>
        <span class="ms-2">{{ local_time }}</span>
      </v-system-bar>
    </v-container>
    <div class="grid-stack">
      <div v-for="(item, index) in widgets" :key="item.id" class="grid-stack-item" :gs-x="item.x" :gs-y="item.y"
        :gs-w="item.w" :gs-h="item.h" :gs-id="item.id" :id="item.id">
        <div class="grid-stack-item-content" :id="item.id">
          <div class="card-header">
            <span>{{ item.config.title }}</span>
            <v-icon icon="mdi-trash-can-outline" class="ms-2" @click="remove(item)" style="float: right;"></v-icon>
            <v-icon icon="mdi-pencil" class="ms-2" @click="edit(item)" style="float: right;"></v-icon>
          </div>
          <div class="card">
            <component :is="grid_kinds[item.kind]" :id="item.id"  :config="item.config"
              v-model:config="widgets[index].config"  @default-config="merge_configs" />
          </div>
        </div>
      </div>
    </div>
    <v-dialog v-model="editDialog" max-width="500" max-height="600">
      <component :is="ConfigEditor" :config="editItem" @default-config="overwrite_configs"
        @close="editDialog = false" />
    </v-dialog>
  </div>

</template>

<script setup>
import { ref, markRaw, onMounted, h, onBeforeUnmount, render, inject, provide, nextTick } from "vue";
import { GridStack } from "gridstack";
// import GridItem from "@/components/GridItem.vue"; // Import your Vue component
import local_bus from "@/LocalBus";

import Gauge from "@/components/Gauge.vue"; // Import your Vue component
import Button from "@/components/Button.vue"; // Import your Vue component
import LineChart from "@/components/LineChart.vue"; // Import your Vue component
import PieChart from "@/components/PieChart.vue"; // Import your Vue component
import Table from "@/components/Table.vue"; // Import your Vue component
import Slider from "@/components/Slider.vue"; // Import your Vue component
import Output from "./Output.vue";
import Progress from "./Progress.vue";
import Switch  from "./Switch.vue";
import YourComponent from "./ConfigEditor.vue";
import ConfigEditor from "./ConfigEditor.vue";


const grid_kinds = {
  Gauge: markRaw(Gauge),
  Button: markRaw(Button),
  LineChart: markRaw(LineChart),
  PieChart: markRaw(PieChart),
  Table: markRaw(Table),
  Slider: markRaw(Slider),
  Output: markRaw(Output),
  Progress:markRaw(Progress),
  Switch:markRaw(Switch),
  ConfigEditor: markRaw(ConfigEditor)
  // Slider: markRaw(() => h('div', 'Slider component not implemented yet')), // Placeholder for Slider
};

let editDialog = ref(false);
let editItem = ref();
function edit(item) {
  editDialog.value = true;
  editItem = item.config;
}

const global = ref({}); // Create a reactive global state
const log_message = ref("log..")
provide('global', global); // Provide global state if needed
let grid = null; // DO NOT use ref(null) as proxies GS will break all logic when comparing structures... see https://github.com/gridstack/gridstack.js/issues/2115
const widgets = ref([
  { x: 0, y: 0, h: 20, w: 2, config: {}, kind: "Gauge", id: "341" },
  { x: 2, y: 0, h: 20, w: 2, config: {}, kind: "LineChart", id: "342" },
  { x: 6, y: 0, h: 10, w: 2, config: {}, kind: "Slider", id: "344" },
  { x: 6, y: 10, h: 10, w: 2, config: {}, kind: "Button", id: "345" },
  { x: 8, y: 0, h: 20, w: 2, config: {}, kind: "Gauge", id: "346" },
]);
const shadowDom = {};

const local_time = ref("")

const v = defineEmits()

function deepMerge(target, source) {
  for (const key in source) {
    if (source[key] instanceof Object && key in target && target[key] instanceof Object) { deepMerge(target[key], source[key]); }
    else { if (!(key in target)) { target[key] = source[key]; } }
  }
  return target;
}

function merge_configs(default_config) {
  console.log("Merging config for id", default_config.id, default_config)
  var widgetIdx = widgets.value.findIndex(widget => widget.id == default_config.id)
  if (widgetIdx != -1) {
    const result = deepMerge(widgets.value[widgetIdx].config, default_config)
  }
}

function deepMergeOverwrite(target, source) {
  for (const key in source) {
    if (source[key] instanceof Object && key in target && target[key] instanceof Object) { deepMergeOverwrite(target[key], source[key]); }
    else { target[key] = source[key]; }
  }
  return target;
}

function overwrite_configs(default_config) {
  console.log("Merging config for id", default_config.id, default_config)
  var widgetIdx = widgets.value.findIndex(widget => widget.id == default_config.id)
  if (widgetIdx != -1) {
    const result = deepMergeOverwrite(widgets.value[widgetIdx].config, default_config)
  }
}



function save() {
  localStorage.setItem('grid1', JSON.stringify(widgets.value));
  console.log("Saved layout:", widgets.value);
}

function load() {
  const saved = localStorage.getItem('grid1');
  console.log("Existing widgets", widgets.value.length);
  while (widgets.value.length > 0) {
    remove(widgets.value[0]);
  }
  
  if (saved) {
    widgets.value = JSON.parse(saved);
    widgets.value.forEach(w => {
      var old_id = w.id
      w.id = String(Math.round(2 ** 32 * Math.random()))
      w.config.id = w.id
      console.log("Changed id from", old_id, "to", w.id)
    }
    );

    // Re-create gridstack widgets
    widgets.value.forEach((item) => {
      nextTick(() => {
        const newEl = document.getElementById(item.id)
        if (newEl)
          grid.makeWidget(newEl);
        else console.warn("Element not found for id", item.id)
      })
    });
  } else {
    console.warn("No saved layout found.");
  }
}

onBeforeUnmount(() => {
  // Clean up Vue renders
  Object.values(shadowDom).forEach((el) => {
    render(null, el);
  });
});

onMounted(() => {
  local_time.value = new Date().toLocaleTimeString('en-US', { hour: 'numeric', hour12: true, minute: 'numeric' });
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
      if (shadowDom[item.id]) {
        render(null, shadowDom[item.id]);
        delete shadowDom[item.id];
      }
    });
  });

  grid.on('change', onChange);

  window.setInterval(() => {
    local_time.value = new Date().toLocaleTimeString('en-US', { hour: 'numeric', hour12: true, minute: 'numeric' });
  }, 3000);


  local_bus.subscribe('some/topic', (data) => {
    console.log('Data received:', data);

  });

});

function onChange(event, changeItems) {
  changeItems.forEach(item => {
    var widget = widgets.value.find(w => w.id == item.id);
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
  var index = widgets.value.findIndex(w => w.id == widget.id);
  if (index !== -1) {
    widgets.value.splice(index, 1);
    const selector = `#${widget.id}`;
    grid.removeWidget(widget.id, true, true);
  } else {
    console.warn("Widget to remove not found in widgets array:", widget);
  }

}


function addWidget(kind) {
  let id = String(Math.round(2 ** 32 * Math.random()))
  let item = {
    h: 20, w: 3, config: {},
    kind: kind, id: id
  };

  // let g = grid.addWidget(item);
  widgets.value.push(item);
  nextTick(() => {
    const newEl = document.getElementById(id)
    if (newEl)
      grid.makeWidget(newEl);
  })
}

/**
 * Deep-merge two Maps without mutating the inputs.
 * - If the same key exists in both:
 *   - Map + Map     => deep-merge Maps
 *   - Object + Obj  => deep-merge plain objects
 *   - Array + Array => concatenate arrays
 *   - Otherwise     => replace with source value (cloned where sensible)
 *
 * @param {Map<any, any>} a
 * @param {Map<any, any>} b
 * @returns {Map<any, any>} new merged Map
 */
function deepMergeMap(a, b) {
  const out = new Map();

  // start with a's entries (cloned)
  for (const [k, v] of a) {
    out.set(k, clone(v));
  }

  // merge/override with b's entries
  for (const [k, v] of b) {
    if (!out.has(k)) {
      out.set(k, clone(v));
      continue;
    }

    const prev = out.get(k);

    if (prev instanceof Map && v instanceof Map) {
      out.set(k, deepMergeMap(prev, v));
    } else if (isPlainObject(prev) && isPlainObject(v)) {
      out.set(k, deepMergeObject(prev, v));
    } else if (Array.isArray(prev) && Array.isArray(v)) {
      out.set(k, prev.concat(v)); // or replace: v.slice()
    } else {
      out.set(k, clone(v)); // fallback: replace
    }
  }

  return out;
}

/* ------------ helpers ------------ */

function isPlainObject(x) {
  return x && Object.prototype.toString.call(x) === '[object Object]';
}

function deepMergeObject(a, b) {
  const out = {};
  // copy a
  for (const k of Reflect.ownKeys(a)) {
    out[k] = clone(a[k]);
  }
  // merge b
  for (const k of Reflect.ownKeys(b)) {
    const av = out[k];
    const bv = b[k];

    if (av instanceof Map && bv instanceof Map) {
      out[k] = deepMergeMap(av, bv);
    } else if (isPlainObject(av) && isPlainObject(bv)) {
      out[k] = deepMergeObject(av, bv);
    } else if (Array.isArray(av) && Array.isArray(bv)) {
      out[k] = av.concat(bv); // or replace: bv.slice()
    } else {
      out[k] = clone(bv);
    }
  }
  return out;
}

function clone(v) {
  if (v == null || typeof v !== 'object') return v;
  if (v instanceof Date) return new Date(v);
  if (v instanceof RegExp) return new RegExp(v.source, v.flags);
  if (Array.isArray(v)) return v.map(clone);
  if (v instanceof Map) return new Map([...v].map(([k, val]) => [k, clone(val)]));
  if (v instanceof Set) return new Set([...v].map(clone));
  if (isPlainObject(v)) {
    const o = {};
    for (const k of Reflect.ownKeys(v)) o[k] = clone(v[k]);
    return o;
  }
  // For other object types (class instances/typed arrays), return as-is.
  return v;
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
  background-color: 	#E0E0E0;
  width: 100% !important;
  height: 100% !important;
}

.grid-stack-item {
  border: 0px solid #000;
}

.handle-remove:hover {
  background-color: #149b80;
}

.card-header {
  margin: 0;
  cursor: move;
  min-height: 15px;
  font-size: 12px;
  background-color: #bdbdbd;
  width: 100%;
}

.card-header:hover {
  background-color: #149b80;
}

.log_message {
  align-content: center;
}


.card {
  width: 100%;
  height: 90%;
}
</style>
