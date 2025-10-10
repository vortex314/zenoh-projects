<template>
    <v-app>
    <div>
        <Grid />
    </div>
    </v-app>
</template>

<script setup lang="ts">
import {  onBeforeUnmount, ref,provide, onMounted } from 'vue'
import  Grid  from './components/Grid.vue'

onMounted(() => {})

onBeforeUnmount(() => {})
</script>

<style scoped></style>