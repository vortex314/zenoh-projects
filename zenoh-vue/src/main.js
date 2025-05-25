//import './assets/main.css'

import { createApp,provide } from 'vue'
import App from './App.vue'

const app = createApp(App);
app.provide('global',{
    message: 'Hello from global provide!'
})
app.mount('#app')

