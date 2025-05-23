import { Redis,Timer } from "./Redis.js";
export  class Sub {
    constructor(pattern, timeout,callback,timeoutCallback) {
        this.pattern = pattern;
        this.timeout = timeout;
        this.callback = callback;
        this.timeoutCallback = timeoutCallback;
        this.timer = new Timer(this.timeout,this.timeoutCallback,true );
        Redis.subscribe(this.pattern, this.callback);
    }
    subscribe(pattern,callback){
        this.pattern = pattern;
        this.callback = callback;
        Redis.subscribe(this.pattern, this.callback);
    }
    unsubscribe(pattern,callback) {
        Redis.unsubscribe(pattern, callback);
//        this.timer.stop();
    }
    resetTimer() {
        this.timer.reset();
    }
    dispose() {
        console.log("disposing sub");
        Redis.unsubscribe(this.pattern, this.callback);
        this.timer.stop();
        this.timer.dispose();
    }
}