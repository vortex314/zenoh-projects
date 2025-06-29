<attachment id="file:/vue-dashboard-app/vue-dashboard-app/src/components/Dashboard.vue">
<template>
  <div class="dashboard">
    <div class="gridstack" ref="gridstack">
      <Widget
        v-for="(widget, index) in widgets"
        :key="widget.id"
        :widget="widget"
        @remove="removeWidget(index)"
      />
    </div>
    <button @click="addWidget">Add Widget</button>
    <button @click="saveLayout">Save Layout</button>
    <button @click="loadLayout">Load Layout</button>
  </div>
</template>

<script>
import { ref, onMounted } from 'vue';
import { loadLayout, saveLayout } from '../services/layout';
import Widget from './Widget.vue';

export default {
  components: {
    Widget,
  },
  setup() {
    const widgets = ref([]);
    const gridstack = ref(null);

    onMounted(() => {
      gridstack.value = GridStack.init();
      loadLayout(widgets);
    });

    const addWidget = () => {
      const newWidget = {
        id: Date.now(),
        type: 'chart', // or 'gauge' based on your needs
        // additional properties for the widget
      };
      widgets.value.push(newWidget);
      gridstack.value.addWidget(newWidget);
    };

    const removeWidget = (index) => {
      widgets.value.splice(index, 1);
      gridstack.value.removeWidget(index);
    };

    const saveLayout = () => {
      saveLayout(widgets.value);
    };

    const loadLayout = () => {
      loadLayout(widgets);
    };

    return {
      widgets,
      gridstack,
      addWidget,
      removeWidget,
      saveLayout,
      loadLayout,
    };
  },
};
</script>

<style scoped>
.dashboard {
  padding: 20px;
}
.gridstack {
  background: #f0f0f0;
}
</style>
</attachment>