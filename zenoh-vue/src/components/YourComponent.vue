<template>
  <v-icon icon="mdi-home" color="primary" size="x-small" />
  <v-btn prepend-icon="mdi-content-save" color="primary" size="x-small" @click="save">Save</v-btn>
  <v-btn prepend-icon="mdi-upload" color="secondary" size="x-small" @click="load">Load</v-btn>
  <v-btn prepend-icon="mdi-chart-line" color="success" size="x-small" @click="showLineChart">Line Chart</v-btn>
  <v-btn prepend-icon="mdi-refresh" color="info" size="x-small" @click="refresh">Refresh</v-btn>
  <v-btn prepend-icon="mdi-gauge" color="warning" size="x-small" @click="showGauge">Gauge</v-btn>
</template>

<script setup>
function save() {
  console.log("Save button clicked!");
  // Add your save logic here
}

function load() {
  console.log("Load button clicked!");
  // Add your load logic here
}

function showLineChart() {
  console.log("Line Chart button clicked!");
  // Add your line chart logic here
}

function refresh() {
  console.log("Refresh button clicked!");
  // Add your refresh logic here
}

function showGauge() {
  console.log("Gauge button clicked!");
  // Add your gauge logic here
}
</script>
