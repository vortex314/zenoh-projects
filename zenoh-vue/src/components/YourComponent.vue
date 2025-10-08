<template>
  <v-data-table :headers="headers" :items="items" item-value="id" class="elevation-1">
    <!-- Editable second column (name) -->
    <template #item.value="{ item }">
      <v-text-field v-model="item.value" variant="solo" density="compact" hide-details />
    </template>
  </v-data-table>
</template>

<script setup>
import { onMounted, ref } from 'vue'

const headers = [
  { title: 'Field', key: 'field' },
  { title: 'Value', key: 'value' },
  { title: 'Type', key: 'type' },
]

const my_object = {
  topic: "src/esp1/motor/MotorInfo/rpm",
  min: 0.1,
  max: 0.2,
  prefix: "prefix",
  suffix: "suffix"
}

const items = ref([]);

onMounted(() => {
  // analyze my_object to items
  for (const [key, value] of Object.entries(my_object)) { items.value.push({ field: key, value: value, type: typeof value }); }
})


</script>