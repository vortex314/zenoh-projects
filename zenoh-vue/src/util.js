import { ref } from vue;

export function setNestedValue(obj, path, value) {
    const keys = path.replace(/\[(\d+)\]/g, '.$1').split('.'); // convert [0] to .0
    const lastKey = keys.pop();

    const target = isRef(obj) ? obj.value : obj;
    let current = target;

    for (const key of keys) {
        // If key is a number, treat current as array
        const isIndex = !isNaN(key);
        if (isIndex) {
            const index = parseInt(key);
            if (!Array.isArray(current)) {
                current = [];
            }
            if (!current[index]) {
                current[index] = {};
            }
            current = current[index];
        } else {
            if (!current[key] || typeof current[key] !== 'object') {
                current[key] = {};
            }
            current = current[key];
        }
    }

    current[lastKey] = value;
}

export function getNestedValue(obj, path) {
    const keys = path.replace(/\[(\d+)\]/g, '.$1').split('.');
    const target = isRef(obj) ? obj.value : obj;

    return keys.reduce((acc, key) => {
        if (acc == null) return undefined;
        return acc[key];
    }, target);
}

/**
 * Syncs specific fields between two reactive objects or refs.
 * @param {Object} source - The source reactive object or ref.
 * @param {Object} target - The target reactive object or ref.
 * @param {Array<{from: string, to: string}>} mappings - Field path mappings.
 */
export function syncMappedFields(source, target, mappings) {
    // const getNestedValue = (obj, path) => path.split('.').reduce((o, key) => o?.[key], obj);
    /*const setNestedValue = (obj, path, value) => {
      const keys = path.split('.');
      const lastKey = keys.pop();
      const nested = keys.reduce((o, key) => o[key], obj);
      nested[lastKey] = value;
    };*/

    console.log("syncMappedFields", mappings);

    mappings.forEach(({ from, to }) => {
        console.log("Mapping ", from, to);
        watch(() => getNestedValue(source.value ?? source, from), (newVal) => {
            setNestedValue(target.value ?? target, to, newVal);
        });

        watch(() => getNestedValue(target.value ?? target, to), (newVal) => {
            setNestedValue(source.value ?? source, from, newVal);
        });
    });
}