<attachment id="file:Widget.vue">
<template>
  <div class="widget" :style="{ width: width + 'px', height: height + 'px' }" @mousedown="startDrag" @mouseup="stopDrag">
    <div class="widget-header">
      <h3>{{ title }}</h3>
      <button @click="removeWidget">X</button>
    </div>
    <div class="widget-content">
      <slot></slot>
    </div>
  </div>
</template>

<script>
export default {
  name: 'Widget',
  props: {
    title: {
      type: String,
      required: true
    },
    width: {
      type: Number,
      default: 300
    },
    height: {
      type: Number,
      default: 200
    }
  },
  methods: {
    startDrag(event) {
      // Logic to initiate dragging
    },
    stopDrag(event) {
      // Logic to stop dragging
    },
    removeWidget() {
      this.$emit('remove');
    }
  }
}
</script>

<style scoped>
.widget {
  border: 1px solid #ccc;
  border-radius: 4px;
  margin: 10px;
  position: relative;
  background-color: #fff;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
}

.widget-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px;
  background-color: #f5f5f5;
  border-bottom: 1px solid #ddd;
}

.widget-content {
  padding: 10px;
}
</style>
</attachment>