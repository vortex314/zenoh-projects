<template>
  <v-card>
    <v-data-table :headers="headers" :items="items" item-value="id" density="compact" class="elevation-1">
      <!-- Editable second column (name) -->
      <template #item.value="{ item }">
        <v-text-field v-model="item.value" variant="solo" density="compact" hide-details />
      </template>
    </v-data-table>
    <!-- save and cancel -->
    <span style="float:right; margin:5px;">
      <v-icon icon="mdi-close" @click="$emit('close')" class="close-icon" />
      <v-icon icon="mdi-content-save" @click="saveConfig" class="save-icon" />
    </span></v-card>
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

const props = defineProps({
  config: {
    type: Object
  },
})

const items = ref([]);
const modelValue = defineModel()  // Vue 3.4+ syntax
const emit = defineEmits(['defaultConfig', 'close'])

onMounted(() => {
  // analyze my_object to items
  for (const [key, value] of Object.entries(props.config)) {
    items.value.push({ field: key, value: value, type: typeof value });
  }
  emit('defaultConfig', props.config);
})

function saveConfig() {
  const newConfig = {};
  items.value.forEach(item => {
    // Convert value to appropriate type
    if (item.type === 'number') {
      newConfig[item.field] = parseFloat(item.value);
    } else if (item.type === 'boolean') {
      newConfig[item.field] = item.value === 'true';
    } else {
      newConfig[item.field] = item.value;
    }
  });
  emit('defaultConfig', newConfig);
  console.log("Saved config:", newConfig);
  emit('close');
}




</script>