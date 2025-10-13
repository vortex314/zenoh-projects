<template>
    <v-app>
        <Grid />
    </v-app>
</template>

<script setup lang="ts">
import {  onBeforeUnmount, ref,provide, onMounted } from 'vue'
import  Grid  from './components/Grid.vue'

onMounted(() => {})

onBeforeUnmount(() => {})
</script>

<style scoped>
html, body, #app {
  height: 100%;
  margin: 0;
  overflow: hidden; /* prevent unwanted global scroll */
}
</style>