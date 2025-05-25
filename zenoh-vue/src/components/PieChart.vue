<template>
    <div>
        <GChart
            type="PieChart"
            :data="[
                ['Label', 'Value'],
                ['Memory', 80],
                ['CPU', 55],
                ['Network', 68],
            ]"
            :options="{
                width: 400,
                height: 120,
                redFrom: 90,
                redTo: 100,
                yellowFrom: 75,
                yellowTo: 90,
                minorTicks: 5
            }"/>

    </div>>
</template>

<script setup lang="ts">

import { ref, onMounted, h, onBeforeUnmount, render } from "vue";
import { GChart } from "vue-google-charts";
// show gauge
import { Gauge } from "vue-google-charts";
const props = defineProps({
  id: {
    type: [String, Number],
    required: true,
    default: "1"
  },
  locked: {
    type: Boolean,
    default: false
  },
  options: {
    type: Object,
    default: () => ({})
  },
  dim: {
    type: Object,
    default: () => ({})
  },
  kind: {
    type: String,
    default: "Gauge"
  }
});
onMounted(() => {
    console.log("Gauge"," component mounted");
});

</script>