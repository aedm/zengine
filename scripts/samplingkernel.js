const _ = require('lodash');

const DISTANCE_RING_COUNT = 32;
const SAMPLES_PER_DISTANCE_RING = 3;
const KERNEL_SIZE = DISTANCE_RING_COUNT * SAMPLES_PER_DISTANCE_RING;

for (let i=0; i<DISTANCE_RING_COUNT; i++) {
  for (let s=0; s<SAMPLES_PER_DISTANCE_RING; s++) {
    const angle = Math.random() * 1000;
    const x = Math.sin(angle) * i;    
    const y = Math.cos(angle) * i;    
    const d = Math.sqrt(x*x + y*y);
    console.log(`  vec3(${x}, ${y}, ${i}),`);
  }
}