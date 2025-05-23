<template>
  <button @click="handleRemove">x</button>
  <a class="handle-remove" @click="handleRemove">x</a>
  <p>Vue Grid Item {{ itemId }}</p>
  <button>Help</button>
</template>

<script setup>
import { toRefs, onBeforeUnmount } from 'vue';

const props = defineProps({
  itemId: {
    type: [String, Number],
    required: true,
  },
});

const emit = defineEmits(['remove']);
const { itemId } = toRefs(props);

onBeforeUnmount(() => {
  console.log(`In vue onBeforeUnmount for item ${itemId.value}`);
});

function handleRemove() {
  emit('remove');
}
</script>

<style scoped>
/* Add your styles here */
.handle-remove:hover {
  background-color: #149b80;
}
</style>
