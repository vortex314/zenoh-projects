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


const removeItem = () => {
    emit('remove', root.value)
}

onBeforeUnmount(() => {
    console.log(`In vue onBeforeUnmount for item ${props.itemId}`)
})
</script>

<style scoped></style>