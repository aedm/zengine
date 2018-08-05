const _ = require('lodash');

const MAX_KERNEL_SIZE = 8;
const VARIANCE = 1.0;

function normalDist(x, sigma) {
  if (sigma == 0) return x === 0 ? 1 : 0;
  return 1.0 / (Math.sqrt(Math.PI * 2) * sigma) * Math.exp(-x*x / (2 * sigma * sigma));
}

for (let i=0; i<MAX_KERNEL_SIZE; i++) {
  const kernel = _.range(MAX_KERNEL_SIZE).map(v => (v > i) ? 0 : normalDist(v, Math.sqrt(i)));
  const sum = kernel.reduce((a, b) => a + b, 0) * 2 - kernel[0];
  const normalizedKernel = kernel.map(v => v / sum);
  console.log(`  { ${normalizedKernel.join(', ')} },`);
}