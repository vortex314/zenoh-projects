<template>
    <div>
        <v-btn size="x-small" class="ma-2" color="primary" @click="items = []">
            Clear Table
        </v-btn>
        <v-table density="compact" striped="even">
            <thead>
                <tr>
                    <th class="left-align">Topic</th>
                    <th class="left-align">Value</th>
                    <th>Time</th>
                    <th>Count</th>
                </tr>
            </thead>
            <tbody class="left-align">
                <tr v-for="item in items" :key="item.name">
                    <td>{{ item.topic }}</td>
                    <td>{{ item.value }}</td>
                    <td>{{ item.date }}</td>
                    <td>{{ item.count }}</td>
                </tr>
            </tbody>
        </v-table>
    </div>
</template>
<script setup>
import bus from "@/LocalBus";
import { ref, onMounted, provide, watch, toRef, isRef, watchEffect } from "vue";
import { defineProps } from "vue";
import "vuetify/styles";
import { VDataTable } from "vuetify/components";

const CONFIG_DEFAULTS = {
    src: "src/**",
    title: "Mains Voltage",
}

const emit = defineEmits(['defaultConfig'])

const props = defineProps({
    id: {
        type: [String, Number],
        required: true,
        default: "1"
    },
    config: {
        type: Object,
        default: () => ({})
    },
});

const items = ref([]);
let subscriber = null;

onMounted(() => {
    CONFIG_DEFAULTS.id = props.id
    emit('defaultConfig', CONFIG_DEFAULTS);
});

watchEffect(() => {
    if (subscriber) subscriber.off();
    if (props.config.src) subscriber = bus.rxd.subscribe(props.config.src, messageHandler);
});

function messageHandler(topic, value) {
    var v = JSON.stringify(value)
    var idx = findIndexOfTopic(topic);
    var ts = new Date().toTimeString().split(' ')[0]
    if (idx < 0) {
        items.value.push({ topic: topic, value: v, date: ts, count: 1 });
    } else {
        var count = items.value[idx].count + 1
        items.value.splice(idx, 1, { topic: topic, value: v, date: ts, count: count });
        //  this.kv[idx]={topic:topic,value :v}; // doesn't work to update Array in vue notifications
    }
}

function findIndexOfTopic(topic) {
    return items.value.findIndex((kvp) => kvp.topic === topic);
}

</script>

<style>
.v-table {
    width: 100%;
    height: 100%;
    background-color: #E0E0E0;
    
}
.left-align {    text-align: left;  }
</style>