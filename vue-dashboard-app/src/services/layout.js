import { reactive } from 'vue';

const layoutKey = 'dashboardLayout';

export const layoutService = {
  saveLayout(layout) {
    localStorage.setItem(layoutKey, JSON.stringify(layout));
  },

  loadLayout() {
    const layout = localStorage.getItem(layoutKey);
    return layout ? JSON.parse(layout) : reactive([]);
  }
};