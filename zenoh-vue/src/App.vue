<template>
    <div>
        <Button />
        <Grid />
    </div>
</template>

<script setup lang="ts">
import { defineProps, onBeforeUnmount, ref } from 'vue'
import Grid from './components/Grid.vue'
import Button from './components/Button.vue'
import 'gridstack/dist/gridstack.min.css';

const props = defineProps({
    title: { type: String, default: "No Title" },
    description: { type: String, default: "No Description" },
    itemId: { type: String, default: "1" },
})

const emit = defineEmits(['remove'])
const root = ref(null)

const removeItem = () => {
    emit('remove', root.value)
}

onBeforeUnmount(() => {
    console.log(`In vue onBeforeUnmount for item ${props.itemId}`)
})
</script>

<style scoped></style>