<template>
    <div>
        <Grid />
    </div>
</template>

<script setup lang="ts">
import { defineProps, onBeforeUnmount, ref,provide } from 'vue'
import Grid from './components/Grid.vue'


const props = defineProps({
    title: { type: String, default: "No Title" },
    description: { type: String, default: "No Description" },
    itemId: { type: String, default: "1" },
})

const emit = defineEmits(['remove'])
const root = ref(null)
let global = {
    cols: 12,
    rows: 12,
    col_width: 100,
    row_height: 100,
};

provide('global', global);

const removeItem = () => {
    emit('remove', root.value)
}

onBeforeUnmount(() => {
    console.log(`In vue onBeforeUnmount for item ${props.itemId}`)
})
</script>

<style scoped></style>