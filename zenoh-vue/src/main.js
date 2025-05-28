//import './assets/main.css'

import { createApp,provide } from 'vue'
import App from './App.vue'
import { PubSub } from './PubSub.js';

import mitt from 'mitt'

const emitter = mitt()


const app = createApp(App);
app.component('pubsub',PubSub)

app.config.globalProperties.emitter = emitter
