const fs = require("fs");
if (process.argv.length < 3) {
	console.log("missing file name");
};
let filename = process.argv[2];
let input = fs.readFileSync(filename, 'utf-8');
let content = JSON.parse(input);
let output = JSON.stringify(content);
fs.writeFileSync(filename + ".zenu", output);
