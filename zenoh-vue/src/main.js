//import './assets/main.css'

import { createApp, provide } from 'vue'
import App from './App.vue'
//import { PubSub } from './PubSub.js';

import emitter from './PubSub.js';
import 'vuetify/styles'
import { createVuetify } from 'vuetify';
import * as components from 'vuetify/components'
import * as directives from 'vuetify/directives'
import { aliases as faAliases, fa } from 'vuetify/iconsets/fa'; // Example for FontAwesome
import { aliases, mdi } from 'vuetify/iconsets/mdi'; // Correct import for mdi icon set
import { VBtn} from 'vuetify/components/VBtn';
import '@mdi/font/css/materialdesignicons.css'; // Ensure the MDI font is imported

import { messageBus } from './PubSub.js';
// import { WebSocket  } from './WebSocket.js';

const vuetify = createVuetify({
  // Specify the components and directives you need
  components : {
    ...components,
    VBtn, // Example of including a specific component
  },
  directives,
  theme: {
    defaultTheme: 'light',
    //
  },
  compilerOptions: {
    isCustomElement: (tag) => {
      return tag.startsWith('v-')
    }
  },
  icons: {
    defaultSet: 'mdi', // Ensure the default icon set is 'mdi'
    aliases, // Use the correct aliases for mdi icons
    sets: {
      mdi, // Ensure the mdi icon set is included
    },
  },

})
// set compilerOptions to use the `v-slot` directive
// const vuetify = createVuetify({
//   components,
//   directives,
//   compilerOptions: {
//     isCustomElement: (tag) => tag.startsWith('v-'),
//   },
const app = createApp(App);
app.use(vuetify);
//app.use(PubSub); // This line is not needed as we are using mitt directly

// app.use(PubSub);
// app.component('pubsub',PubSub)

app.config.globalProperties.emitter = emitter
console.log("emitter", emitter)
app.mount('#app');
