<template>
    <div>
        <v-btn class="ma-2" color="primary" @click="items = []">
            Clear Table
        </v-btn>
        <v-data-table :headers="headers" :items="items">
        </v-data-table>
    </div>
</template>
<script setup>
import { messageBus } from "@/PubSub";
import { onMounted, ref } from "vue";
import { defineProps } from "vue";
import "vuetify/styles";
import { VDataTable } from "vuetify/components";


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
    dim: {
        type: Object,
        default: () => ({})
    }
});

const headers = ref([
    { title: "Topic", key: "topic", class: "blue lighten-5" },
    { title: "key", key: "value", class: "blue lighten-5" },
    { title: "Date", key: "date", class: "blue lighten-5" },
    { title: "Count", key: "count", class: "blue lighten-5" }
]);

const items = ref([]);

onMounted(() => {
    messageBus.listen("*", (msg) => {
        console.log("SubTable received message:", msg);
        onMessage(msg.topic, msg.value);
    });
});


function onMessage(topic, message) {
    var v = JSON.stringify(message)
    var idx = findIndexOfTopic(topic);
    console.log("SubTable.onMessage idx:" + idx + " topic:" + topic + " message:" + v + " length " + items.value.length);
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
.v-data-table {
    width: 100%;
    height: 100%;
}
</style>